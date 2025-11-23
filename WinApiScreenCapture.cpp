#include "WinApiScreenCapture.h"
#include <string>

static void logLastError(const std::string& context) {

    auto err = GetLastErrorAsString();
    if (!err.empty()) {
        logError(context + " failed. Error: " + err);
        return;
    }
}

WinApiScreenCapture::WinApiScreenCapture()
{
    hScreenDC = GetDC(nullptr);
    if (!hScreenDC) {
        logLastError("GetDC");
    } 

    hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        logLastError("CreateCompatibleDC");
    } 
}

WinApiScreenCapture::~WinApiScreenCapture()
{
    if (hScreenDC) {
        ReleaseDC(nullptr, hScreenDC);
    }
    if (hMemoryDC) {
        DeleteDC(hMemoryDC);
    }
}

cv::Mat HBitmapToMat(HDC hdc, HBITMAP hBitmap)
{
    BITMAP bmp;
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) {
        logLastError("GetObject");
        return cv::Mat();
    }

    int imgWidth = bmp.bmWidth;
    int imgHeight = bmp.bmHeight;
    if (imgWidth <= 0 || imgHeight <= 0) {
        logError("Invalid bitmap dimensions: " + std::to_string(imgWidth) + "x" + std::to_string(imgHeight));
        return cv::Mat();
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = imgWidth;
    bmi.bmiHeader.biHeight = -imgHeight; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    cv::Mat image(imgHeight, imgWidth, CV_8UC3);
    int res = GetDIBits(hdc, hBitmap, 0, imgHeight, image.data, &bmi, DIB_RGB_COLORS);
    if (res == 0) {
        logLastError("GetDIBits");
        return cv::Mat();
    }

    return image;
}

auto WinApiScreenCapture::grabScreen(RECT region) -> std::optional<cv::Mat>
{
    int width = region.right - region.left;
    int height = region.bottom - region.top;
    if (width <= 0 || height <= 0) {
        logError("Invalid capture region dimensions: " + std::to_string(width) + "x" + std::to_string(height));
        return std::nullopt;
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        logLastError("CreateCompatibleBitmap");
        return std::nullopt;
    }

    if (!SelectObject(hMemoryDC, hBitmap)) {
        logLastError("SelectObject");
        DeleteObject(hBitmap);
        return std::nullopt;
    }

    if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, region.left, region.top, SRCCOPY)) {
        logLastError("BitBlt");
        DeleteObject(hBitmap);
        return std::nullopt;
    }

    cv::Mat img = HBitmapToMat(hMemoryDC, hBitmap);
    DeleteObject(hBitmap);

    if (img.empty()) {
        logError("Failed to convert captured bitmap to cv::Mat");
        return std::nullopt;
    }

    return img;
}
