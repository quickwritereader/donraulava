// donRaulAva.cpp : Defines the entry point for the application.
//

#include "donRaulAva.h"
#include "DetectLoop.h"
// Global Variables:

Gdiplus::Bitmap *bitmap;
Gdiplus::Bitmap *glowBitmap;
RECT ScreenRect;
float ScaleFactor;
DetectLoop detectLoop;

auto checkCloseBtnAnimation(HWND hWnd, Gdiplus::Bitmap *bitmap, const POINT &currentPos) -> void;
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    using namespace Gdiplus;
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // Define a unique name for the mutex
    auto mutexName = "DONRAULITO_MUTEX_ONCE_ONCE";

    // Attempt to create a mutex
    HANDLE hMutex = CreateMutex(NULL, TRUE, mutexName);

    // Check if the mutex already exists
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // Another instance is already running
        MessageBoxA(NULL, "Another instance of this program is already running.", "Warning", MB_OK | MB_ICONWARNING);
        return 0;
    }

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Load the image
    // Load the PNG image using GDI+
    bitmap = LoadBitmapFromResource(hInstance, MAKEINTRESOURCE(IDR_RAUL_PNG), "PNG");
    // Get the bitmap size
    LONG width = (LONG)bitmap->GetWidth();
    LONG height = (LONG)bitmap->GetHeight();
    // Create a copy of the image
    glowBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
    Graphics glowGraphics(glowBitmap);
    glowGraphics.DrawImage(bitmap, 0, 0, width, height);
    ApplyGaussianBlurTint(glowBitmap, 13, Color(255, 255, 0, 255));
    // draw original over again
    glowGraphics.DrawImage(bitmap, 0, 0, width, height);
    // draw close button on both bitmaps
    DrawCloseBtnOnBitmap(bitmap, width, height, false);
    DrawCloseBtnOnBitmap(glowBitmap, width, height, false);

    if (bitmap->GetLastStatus() != Ok)
    {
        logError("Bitmap load failure", "DonRaulito");
        return 0;
    }

    // Register the window class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "DonRaulito";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        logError("Call to RegisterClassEx failed!", "DonRaulito");
        return 1;
    }
   

    // Note: The lambda function captures the static storage duration variable 'detectLoop' by reference.
    // This ensures that the lambda has access to the same instance of 'detectLoop' throughout its execution.
    // Capturing by reference is crucial here because 'detectLoop' is a global variable and we want to
    // operate on the same instance across different parts of the program.
    std::jthread detectThread([](std::stop_token stopToken, DetectLoop &dlooper) mutable {
        dlooper.loop(stopToken);
    },std::ref(detectLoop));
 
    // Create the window
    HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, wcex.lpszClassName, TEXT("Don Raulito"),
                               WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
                               width, height, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        logError("Call to CreateWindow failed!", "DonRaulito");
        return 1;
    }

    std::tie(ScreenRect, ScaleFactor) = GetDesktopScreenRect();
    logInfo("Screen Information", "Scale Factor: ", ScaleFactor, "Screen Width: ", ScreenRect.right,
             "Screen Height: ", ScreenRect.bottom);
    DrawWindow(hWnd, bitmap, ScreenRect.right - width - 80, ScreenRect.bottom - height - 120, width, height);
    ShowWindow(hWnd, nCmdShow);
    // Show the window

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Release the mutex when the program exits
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    //notify our loop for end
    detectLoop.stop();
    detectThread.request_stop();
    detectThread.join();
    delete bitmap;
    delete glowBitmap;
    return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//  PURPOSE:  Processes messages for the main window.
LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    static HINSTANCE glInstance = nullptr;
    static ConfigDialog *config = nullptr;
    static bool dragging = false;
    static bool position_changed = false;
    static POINT dragStartPoint;
    static uint64 startTick = 0;
    static Gdiplus::Bitmap *currentBitmap = bitmap;
    switch (message)
    {
    case WM_CREATE:
        glInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
        config = new ConfigDialog(glInstance, hWnd);
        break;
    // explicit dragging without using NCHITTEST overriding
    case WM_LBUTTONDOWN:
    {
        dragging = true;
        SetCapture(hWnd);
        dragStartPoint.x = LOWORD(lParam);
        dragStartPoint.y = HIWORD(lParam);
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        POINT currentPos;
        currentPos.x = LOWORD(lParam);
        currentPos.y = HIWORD(lParam);
        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);
        auto width = windowRect.right - windowRect.left;
        auto height = windowRect.bottom - windowRect.top;

        if (dragging)
        {

            // Calculate the new position
            int newX = windowRect.left + (currentPos.x - dragStartPoint.x);
            int newY = windowRect.top + (currentPos.y - dragStartPoint.y);

            // Move the window
            SetWindowPos(hWnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            position_changed = true;
        }
        else
        {
            checkCloseBtnAnimation(hWnd, currentBitmap, currentPos);
        }
        return 0;
    }
    case WM_LBUTTONUP:
    {
        dragging = false;
        ReleaseCapture();
        if (!position_changed)
        {
            // Get the cursor position
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            RECT windowRect;
            GetWindowRect(hWnd, &windowRect);
            auto width = windowRect.right - windowRect.left;
            auto height = windowRect.bottom - windowRect.top;

            if (withinClosebtn(hWnd, {xPos, yPos}, width, height))
            {
                // Close the window
                // Simulate close button click
                PostMessage(hWnd, WM_CLOSE, 0, 0);
            }
            else
            {
                
                // Simulate double-click
                if (CurrentMilliseconds() - startTick < 1000)
                {
                    currentBitmap = currentBitmap == bitmap ? glowBitmap : bitmap;
                    DrawWindow(hWnd, currentBitmap, windowRect.left, windowRect.top, width, height);
                    if(currentBitmap == glowBitmap){
                        detectLoop.setParameters(ScreenRect, *config, true);
                        detectLoop.start();
                    }else{
                        detectLoop.stop();
                    }
                }
                startTick = CurrentMilliseconds();
            }
        }
        position_changed = false;
    }
        return 0;
    case WM_RBUTTONUP:
        config->open();
        break;
    case WM_DESTROY:
        logInfo("Destroying window", "DonRaulito");
        PostQuitMessage(0);
        delete config;
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

auto checkCloseBtnAnimation(HWND hWnd, Gdiplus::Bitmap *bitmap, const POINT &currentPos) -> void
{
    static bool closebtnUpdated = false;
    RECT windowRect;
    GetWindowRect(hWnd, &windowRect);
    auto width = windowRect.right - windowRect.left;
    auto height = windowRect.bottom - windowRect.top;
    bool within = withinClosebtn(hWnd, currentPos, width, height);
    if (!closebtnUpdated && within)
    {
        // Set the capture to the window
        // This way we can track mouse events outside the window
        // another option would be to use TrackMouseEvent
        SetCapture(hWnd);
        closebtnUpdated = true;
        DrawCloseBtnOnBitmap(bitmap, width, height, true);
        DrawWindow(hWnd, bitmap, windowRect.left, windowRect.top, width, height);
    }
    else if (closebtnUpdated && !within)
    {
        ReleaseCapture();
        closebtnUpdated = false;
        DrawCloseBtnOnBitmap(bitmap, width, height, false);
        DrawWindow(hWnd, bitmap, windowRect.left, windowRect.top, width, height);
    }
}
