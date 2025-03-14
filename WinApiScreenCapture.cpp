#include "WinApiScreenCapture.h"

WinApiScreenCapture::WinApiScreenCapture()
{
    // Initialize DCs
    hScreenDC = GetDC(nullptr);
    hMemoryDC = CreateCompatibleDC(hScreenDC);
}

WinApiScreenCapture::~WinApiScreenCapture()
{
    // Release DCs
    ReleaseDC(nullptr, hScreenDC);
    DeleteDC(hMemoryDC);
    logInfo("WinApiScreenCapture destructor called");
}

cv::Mat HBitmapToMat(HDC hdc, HBITMAP hBitmap)
{
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int imgWidth = bmp.bmWidth;
    int imgHeight = bmp.bmHeight;


    // Retrieve the bitmap bits as grayscale
   // Create a BITMAPINFO structure
        BITMAPINFO bmi;
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = imgWidth;
        bmi.bmiHeader.biHeight = -imgHeight; // Negative height to indicate top-down bitmap
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;
        bmi.bmiHeader.biXPelsPerMeter = 0;
        bmi.bmiHeader.biYPelsPerMeter = 0;
        bmi.bmiHeader.biClrUsed = 0;
        bmi.bmiHeader.biClrImportant = 0;

        // Create a buffer to hold the bitmap data
        cv::Mat image(imgHeight, imgWidth, CV_8UC3);
        GetDIBits(hdc, hBitmap, 0, imgHeight, image.data, &bmi, DIB_RGB_COLORS);
        return image;

}

auto WinApiScreenCapture::grabScreen(RECT region) -> cv::Mat
{
    // Capture screen using Win32 API
    int width = region.right - region.left;
    int height = region.bottom - region.top;

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, region.left, region.top, SRCCOPY);

    // Convert Win32 HBITMAP to OpenCV image
    cv::Mat img = HBitmapToMat(hMemoryDC, hBitmap);

    // Release the bitmap
    DeleteObject(hBitmap);

    return img;
}
