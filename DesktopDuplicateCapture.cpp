#include "DesktopDuplicateCapture.h"


DesktopDuplicationCapture::DesktopDuplicationCapture(UINT OutputNumber) {
    Initialize(OutputNumber);
}

DesktopDuplicationCapture::~DesktopDuplicationCapture() {
    Close();
}

auto DesktopDuplicationCapture::Initialize(UINT OutputNumber) ->void{
    HRESULT hr = S_OK;

    // Driver types supported
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    auto numDriverTypes = ARRAYSIZE(driverTypes);

    // Feature levels supported
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1,
    };
    auto numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel;

    // Create device
    for (size_t i = 0; i < numDriverTypes; i++) {
        hr = D3D11CreateDevice(nullptr, driverTypes[i], nullptr, 0, featureLevels, (UINT)numFeatureLevels,
                               D3D11_SDK_VERSION, &D3DDevice, &featureLevel, &D3DDeviceContext);
        if (SUCCEEDED(hr)) break;
    }
    if (FAILED(hr)) {
        logError("D3D11CreateDevice failed", hr);
    }

    // Get DXGI device
    IDXGIDevice* dxgiDevice = nullptr;
    hr = D3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) {
        logError("D3DDevice->QueryInterface failed", hr);
    }

    // Get DXGI adapter
    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
    dxgiDevice->Release();
    dxgiDevice = nullptr;
    if (FAILED(hr)) {
        logError("dxgiDevice->GetParent failed", hr);
    }

    // Get output
    IDXGIOutput* dxgiOutput = nullptr;
    hr = dxgiAdapter->EnumOutputs(OutputNumber, &dxgiOutput);
    dxgiAdapter->Release();
    dxgiAdapter = nullptr;
    if (FAILED(hr)) {
        logError("dxgiAdapter->EnumOutputs failed", hr);
    }

    dxgiOutput->GetDesc(&OutputDesc);

    // Get IDXGIOutput1
    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgiOutput1);
    dxgiOutput->Release();
    dxgiOutput = nullptr;
    if (FAILED(hr)) {
        logError("dxgiOutput->QueryInterface failed", hr);
    }

    // Create desktop duplication
    hr = dxgiOutput1->DuplicateOutput(D3DDevice, &DeskDupl);
    dxgiOutput1->Release();
    dxgiOutput1 = nullptr;
    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
            logError("Too many desktop recorders already active");
        } else {
            logError("DuplicateOutput failed", hr);
        }
    }

}

auto DesktopDuplicationCapture::Close()->void {
    if (DeskDupl) DeskDupl->Release();
    if (D3DDeviceContext) D3DDeviceContext->Release();
    if (D3DDevice) D3DDevice->Release();

    DeskDupl = nullptr;
    D3DDeviceContext = nullptr;
    D3DDevice = nullptr;
    HaveFrameLock = false;
}

auto DesktopDuplicationCapture::CaptureNext(const RECT& region) ->bool{
    if (!DeskDupl) return false;

    HRESULT hr;

    if (HaveFrameLock) {
        HaveFrameLock = false;
        hr = DeskDupl->ReleaseFrame();
        if (FAILED(hr)) {
            logError("ReleaseFrame failed", hr);
        }
    }

    IDXGIResource* deskRes = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    hr = DeskDupl->AcquireNextFrame(0, &frameInfo, &deskRes);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) return false;
    if (FAILED(hr)) {
        logError("AcquireNextFrame failed", hr);
        return false;
    }

    HaveFrameLock = true;

    ID3D11Texture2D* gpuTex = nullptr;
    hr = deskRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&gpuTex);
    deskRes->Release();
    deskRes = nullptr;
    if (FAILED(hr)) {
        logError("QueryInterface for ID3D11Texture2D failed", hr);
        return false;
    }

    D3D11_TEXTURE2D_DESC desc;
    gpuTex->GetDesc(&desc);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;  // Setting MiscFlags to 0
    ID3D11Texture2D* cpuTex = nullptr;
    logInfo("Texture description: Width =", desc.Width, "Height =", desc.Height, "Format =", desc.Format, "ArraySize =", desc.ArraySize, "MipLevels =", desc.MipLevels, "SampleDesc.Count =", desc.SampleDesc.Count, "SampleDesc.Quality =", desc.SampleDesc.Quality, "CPUAccessFlags =", desc.CPUAccessFlags, "Usage =", desc.Usage, "BindFlags =", desc.BindFlags, "MiscFlags =", desc.MiscFlags);

    hr = D3DDevice->CreateTexture2D(&desc, nullptr, &cpuTex);
    if (FAILED(hr)) {
        logError("CreateTexture2D failed", hr);
        return false;
    }

    D3DDeviceContext->CopyResource(cpuTex, gpuTex);

    D3D11_MAPPED_SUBRESOURCE sr;
    hr = D3DDeviceContext->Map(cpuTex, 0, D3D11_MAP_READ, 0, &sr);
    if (SUCCEEDED(hr)) {
        int regionWidth = region.right - region.left;
        int regionHeight = region.bottom - region.top;

        if (Latest.Width != regionWidth || Latest.Height != regionHeight) {
            Latest.Width = regionWidth;
            Latest.Height = regionHeight;
            Latest.Buf.resize(regionWidth * regionHeight * 4);
        }

        for (int y = 0; y < regionHeight; y++) {
            memcpy(Latest.Buf.data() + y * regionWidth * 4,
                   (uint8_t*)sr.pData + (region.top + y) * sr.RowPitch + region.left * 4,
                   regionWidth * 4);
        }
        D3DDeviceContext->Unmap(cpuTex, 0);
    } else {
        logError("Map failed", hr);
        return false;
    }

    cpuTex->Release();
    gpuTex->Release();

    return true;
}

auto DesktopDuplicationCapture::grabScreen(RECT region) ->cv::Mat{
    CaptureNext(region);
    return cv::Mat(Latest.Height, Latest.Width, CV_8UC4, (void*)Latest.Buf.data()).clone();
}


