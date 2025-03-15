#include "DetectLoop.h"
#include "WinApiScreenCapture.h"
#include "DesktopDuplicateCapture.h"
#include <filesystem>
#include <optional>

constexpr int MINIMUM_LINE_LENGTH = 170;
constexpr int BORDER_MATCH_COUNT = 7;

// Function to check if two borders are within a certain limit
auto withinLimit(const RECT &border1, const RECT &border2) -> bool
{
    constexpr int LIMIT = 20; // Define a limit value
    return (abs(border1.left - border2.left) < LIMIT &&
            abs(border1.top - border2.top) < LIMIT &&
            abs(border1.right - border2.right) < LIMIT &&
            abs(border1.bottom - border2.bottom) < LIMIT);
}

auto doubleRectCoords(const std::optional<RECT>& rectOpt)->std::optional<RECT>{
    if (rectOpt.has_value()) {
        RECT rect = rectOpt.value();
        rect.left *= 2;
        rect.top *= 2;
        rect.right *= 2;
        rect.bottom *= 2;
        return rect;
    }
    return std::nullopt;
}



auto DetectLoop::loop(std::stop_token stopToken) -> void
{

    std::unique_ptr<ScreenCapture> screenCapture; 

    // Create tempCaptures folder path to log pictures
    std::string path = getUserProfile() + "\\tempCaptures";
    auto matchScale=1.0;
    cv::Mat leftTemplate, rightTemplate, upTemplate, downTemplate;
    cv::cvtColor(trackObject, upTemplate, cv::COLOR_BGR2GRAY);
    cv::rotate(upTemplate, leftTemplate, cv::ROTATE_90_COUNTERCLOCKWISE);
    cv::rotate(upTemplate, rightTemplate, cv::ROTATE_90_CLOCKWISE);
    cv::rotate(leftTemplate, downTemplate, cv::ROTATE_90_COUNTERCLOCKWISE);


    while (!stopToken.stop_requested())
    {
        logInfo("DetectLoop loop outer loop Id", id);
        // Dispose Capture screen
        screenCapture = nullptr;

        // wait for semaphore
        if (!sem.try_acquire_for(std::chrono::milliseconds(10000)))
        {
            logError("Failed to acquire semaphore");
            continue;
        }
        // local copy of renewed atomic variables
        RECT rect = {left, top, right, bottom};
        RECT border = {};
        int borderDetectionCount = 0;
        bool saveForDebug = saveImagesAndTracks;
        int dc = 0;
        constexpr int DCMAX = 100;
        int fps = 0;
        auto totalElapsed = 0.0;
        while (m_loop && !stopToken.stop_requested())
        {
            auto start=CurrentMilliseconds();
            if (!screenCapture)
            {
                logInfo("DetectLoop loop inner loop Screen Capture init", captureMethod);
                // Create the screen capture object based on the capture method
                if (captureMethod == 1)
                {
                    screenCapture = std::make_unique<WinApiScreenCapture>();
                }
                else
                {
                    screenCapture = std::make_unique<DesktopDuplicationCapture>();
                }
                // Create tempCaptures folder if it does not exist
                if (saveForDebug && !std::filesystem::exists(path))
                {
                    std::filesystem::create_directory(path);
                }
            }

            auto screen = screenCapture->grabScreen(rect);

            cv::Mat grayScreen;
            cv::cvtColor(screen, grayScreen, cv::COLOR_BGR2GRAY);
            std::optional<RECT> potentialBorder = std::nullopt;
            Sleep(1);
            if(borderDetectionCount<=BORDER_MATCH_COUNT) {
                // Downsample the image
                cv::resize(grayScreen, grayScreen, cv::Size(), 0.5, 0.5); 
                potentialBorder = detectBorder(grayScreen, MINIMUM_LINE_LENGTH);
                potentialBorder = doubleRectCoords(potentialBorder);
            }else{

            }
            
            if (potentialBorder)
            {

                if (withinLimit(border, *potentialBorder))
                {
                    ++borderDetectionCount;
                }
                else
                {
                    border = *potentialBorder;
                    borderDetectionCount = 1;
                }

                if (borderDetectionCount > BORDER_MATCH_COUNT)
                {
                    // capture borders from now on
                    rect.left += border.left;
                    rect.top += border.top;
                    rect.right = rect.left + (border.right - border.left);
                    rect.bottom = rect.top + (border.bottom - border.top);

                    logInfo("New Grab Rect after Border detection", rect.left, rect.top, rect.right, rect.bottom);
                    if (saveForDebug)
                    {
                        dc = dc % DCMAX;
                        if(potentialBorder){
                            cv::Rect cvRect(potentialBorder->left, potentialBorder->top, potentialBorder->right - potentialBorder->left, potentialBorder->bottom - potentialBorder->top);
                            cv::rectangle(screen, cvRect, cv::Scalar(0,0,255),2);
                        } 
                        cv::imwrite(path + "\\screen" + std::to_string(dc) + ".jpg", screen);
                        dc++;
                    }
                }
            }

            auto elapsed = CurrentMilliseconds() -start;
            totalElapsed +=elapsed;
            if (totalElapsed >= 10000)
            {
                logInfo("FPS", fps / 10.0);
                totalElapsed = 0;
                fps = 0;
            } 
            //downGrade Fps to reduce cpu usage
            if (elapsed<55) Sleep(55);
            else Sleep(10);
            fps++;
        }

        Sleep(1000);
    }
}
