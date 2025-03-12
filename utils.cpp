#include "utils.h"

using namespace Gdiplus;

auto getUserProfile() -> std::string
{
    char userProfile[MAX_PATH];
    DWORD size = sizeof(userProfile);
    GetEnvironmentVariable("USERPROFILE", userProfile, size);
    return std::string(userProfile);
}

auto  safeStoiDefault(const std::string &text, int defaultVal) -> int
{
	try
	{
		return std::stoi(text);
	}
	catch (...)
	{
		return defaultVal;
	}
}

auto DrawCloseBtn(Gdiplus::Bitmap *bitmap, LONG width, LONG height, bool mouseOver = false) -> void
{
    // Draw our close rectangle on bitmap
    Graphics bmpGraphics(bitmap);
    SolidBrush brush(mouseOver ? Color(255, 255, 0, 0) : Color(255, 0, 0, 255));
    Pen p(Color(255, 255, 255, 255), 2);
    bmpGraphics.FillRectangle(&brush, width - width / 10, 0, width, height / 10);
    bmpGraphics.DrawLine(&p, width - width / 10, 0, width, height / 10);
    bmpGraphics.DrawLine(&p, width - width / 10, height / 10, width, 0);
}

auto DrawWindow(HWND hWnd, LONG x, LONG y, LONG &width, LONG &height, Gdiplus::Bitmap *&bitmap) -> void
{

    // Create a compatible DC and bitmap
    HDC hdcScreen = GetDC(hWnd);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBitmap);

    // Draw the GDI+ bitmap to the compatible DC
    Graphics graphics(hdcMem);
    graphics.DrawImage(bitmap, 0, 0, width, height);

    // Prepare the BLENDFUNCTION structure
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255; // No opacity change
    blend.AlphaFormat = AC_SRC_ALPHA;

    // Update the layered window
    POINT ptSrc = {0, 0};
    SIZE size = {width, height};
    POINT ptDst = {x, y};

    UpdateLayeredWindow(hWnd, hdcScreen, &ptDst, &size, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);
}

auto withinClosebtn(HWND hWnd, const POINT &currentPos, LONG width, LONG height) -> bool
{

    return currentPos.x > width - width / 10 && currentPos.x < width &&
           currentPos.y > 0 && currentPos.y < height / 10;
}

auto ApplyGaussianBlurTint(Bitmap *bitmap, int blurRadius, Color tint) ->void
{
    int width = bitmap->GetWidth();
    int height = bitmap->GetHeight();
    BitmapData bitmapData;
    Rect rect(0, 0, width, height);

    if (bitmap->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData) == Ok)
    {
        int stride = bitmapData.Stride;
        BYTE *pixels = static_cast<BYTE *>(bitmapData.Scan0);

        // Temporary buffer to store blurred pixels
        std::vector<BYTE> tempPixels(width * height * 4);

        // Apply blur to each pixel
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int red = 0, green = 0, blue = 0, alpha = 0;
                int count = 0;

                for (int ky = -blurRadius; ky <= blurRadius; ++ky)
                {
                    for (int kx = -blurRadius; kx <= blurRadius; ++kx)
                    {
                        int nx = x + kx;
                        int ny = y + ky;

                        if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                        {
                            int pixelIndex = (ny * stride) + (nx * 4);
                            blue += pixels[pixelIndex];
                            green += pixels[pixelIndex + 1];
                            red += pixels[pixelIndex + 2];
                            alpha += pixels[pixelIndex + 3];
                            count++;
                        }
                    }
                }

                int tempIndex = (y * width + x) * 4;
                tempPixels[tempIndex] = static_cast<BYTE>(blue / count);
                tempPixels[tempIndex + 1] = static_cast<BYTE>(green / count);
                tempPixels[tempIndex + 2] = static_cast<BYTE>(red / count);
                tempPixels[tempIndex + 3] = static_cast<BYTE>(alpha / count);
            }
        }

        // Copy blurred pixels back to the bitmap
        memcpy(pixels, tempPixels.data(), width * height * 4);

        // Apply neon tint to each pixel
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int pixelIndex = (y * stride) + (x * 4);
                BYTE blue = pixels[pixelIndex];
                BYTE green = pixels[pixelIndex + 1];
                BYTE red = pixels[pixelIndex + 2];
                BYTE alpha = pixels[pixelIndex + 3];

                // Mix the original color with the tint color
                pixels[pixelIndex] = (blue + tint.GetB()) / 2;
                pixels[pixelIndex + 1] = (green + tint.GetG()) / 2;
                pixels[pixelIndex + 2] = (red + tint.GetR()) / 2;
                pixels[pixelIndex + 3] = alpha; // Preserve the alpha channel
            }
        }

        bitmap->UnlockBits(&bitmapData);
    }
}

// Function to simulate a key press and release for any key
auto SimulateKeyPress(WORD virtualKey)->void {
    // Create an array of INPUT structures
    INPUT inputs[2] = {};

    // Set up the INPUT structure for the key press
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = virtualKey; // Virtual-key code for the key press
    inputs[0].ki.dwFlags = 0; // 0 for key press

    // Set up the INPUT structure for the key release
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = virtualKey; // Virtual-key code for the key release
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release

    // Send the inputs
    SendInput(2, inputs, sizeof(INPUT));
}