#include "WinApiScreenCapture.h"
#include <string>
#include <optional>
#include <windows.h>
#include <opencv2/opencv.hpp>



static void logLastError(const std::string& context) {
    DWORD code = GetLastError();
    if (code != 0) {
        auto err = GetLastErrorAsString();
        logError(context + " failed. Error: " + err);
        SetLastError(0);
    }
}
 
WinApiScreenCapture::WinApiScreenCapture()
{
 
    hScreenDC = GetDC(nullptr);
    if (!hScreenDC) {
        logLastError("GetDC(nullptr)");
    }
    hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        logLastError("CreateCompatibleDC");
    }
}

WinApiScreenCapture::~WinApiScreenCapture()
{
    if (hMemoryDC) {
        DeleteDC(hMemoryDC);
    }
    if (hScreenDC) {
        ReleaseDC(nullptr, hScreenDC);
    }
}
 

// Convert HBITMAP to cv::Mat using 32-bit BGRA and convert to BGR.
static cv::Mat HBitmapToMat32(HDC hdc, HBITMAP hBitmap)
{
    BITMAP bmp{};
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0) {
        logLastError("GetObject(HBITMAP)");
        return {};
    }

    const int imgWidth  = bmp.bmWidth;
    const int imgHeight = bmp.bmHeight;
    if (imgWidth <= 0 || imgHeight <= 0) {
        logError(std::string("Invalid bitmap dimensions: ")
                 + std::to_string(imgWidth) + "x" + std::to_string(imgHeight));
        return {};
    }

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = imgWidth;
    bmi.bmiHeader.biHeight      = -imgHeight; // top-down
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;         // BGRA
    bmi.bmiHeader.biCompression = BI_RGB;

    cv::Mat bgra(imgHeight, imgWidth, CV_8UC4);
    const int scanLines = GetDIBits(hdc, hBitmap, 0, imgHeight, bgra.data, &bmi, DIB_RGB_COLORS);
    if (scanLines == 0) {
        logLastError("GetDIBits");
        return {};
    }

    cv::Mat bgr;
    cv::cvtColor(bgra, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}

std::optional<cv::Mat> WinApiScreenCapture::grabScreen(RECT region)
{
    if (!hScreenDC || !hMemoryDC) {
        logError("Device contexts are not initialized.");
        return std::nullopt;
    }

    int width  = region.right  - region.left;
    int height = region.bottom - region.top;

    if (width <= 0 || height <= 0) {
        logError(std::string("Invalid capture region dimensions: ")
                 + std::to_string(width) + "x" + std::to_string(height));
        return std::nullopt;
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        logLastError("CreateCompatibleBitmap");
        return std::nullopt;
    }

    HGDIOBJ oldObj = SelectObject(hMemoryDC, hBitmap);
    if (!oldObj || oldObj == HGDI_ERROR) {
        logLastError("SelectObject(hBitmap)");
        DeleteObject(hBitmap);
        return std::nullopt;
    }

    if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, region.left, region.top, SRCCOPY)) {
        logLastError("BitBlt");
        SelectObject(hMemoryDC, oldObj);
        DeleteObject(hBitmap);
        return std::nullopt;
    }

    cv::Mat img = HBitmapToMat32(hMemoryDC, hBitmap);

    SelectObject(hMemoryDC, oldObj);
    DeleteObject(hBitmap);

    if (img.empty()) {
        logError("Failed to convert captured bitmap to cv::Mat");
        return std::nullopt;
    }

    return img;
}
