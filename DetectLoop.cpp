#include "DetectLoop.h"
#include "WinApiScreenCapture.h"
#include "DesktopDuplicateCapture.h"
#include <filesystem>
#include <fstream>
#include <optional>
#include <opencv2/core/ocl.hpp>
#include <stop_token>
#include "NaiveTracker.h"

constexpr int MINIMUM_LINE_LENGTH = 170;
constexpr int BORDER_MATCH_COUNT = 7;
constexpr int DETECT_AREA_HEIGHT = 98;
constexpr double NO_OCCULSION_THRESHOLD = 0.55;

// Function to check if two borders are within a certain limit
auto withinLimit(const RECT &border1, const RECT &border2) -> bool
{
    constexpr int LIMIT = 20; // Define a limit value
    return (abs(border1.left - border2.left) < LIMIT &&
            abs(border1.top - border2.top) < LIMIT &&
            abs(border1.right - border2.right) < LIMIT &&
            abs(border1.bottom - border2.bottom) < LIMIT);
}

auto getLocationsBottomY(const cv::Mat &result, int height, int exitArea, float threshold = NO_OCCULSION_THRESHOLD) -> std::vector<int>
{
    std::vector<int> locations;
    constexpr int dispY = 17;
    locations.reserve(50);
    float last_thresh = 0;
    int lastY = 0; 
    for (int y = 0; y < result.rows; ++y)
    {
        for (int x = 0; x < result.cols; ++x)
        {
            auto thresh = result.at<float>(y, x);
            if (thresh >= threshold)
            {
                // do not match
                if(y>exitArea) {
                    continue;
                }
                if (std::abs(y - lastY) > dispY)
                {
                    locations.emplace_back(y + height);
                    lastY = y;
                    last_thresh = thresh;
                }
                else
                {
                    if (thresh > last_thresh)
                    {  

                        if(locations.size()>0){
                            locations.back() = y + height;
                        }else{
                            locations.emplace_back(y + height);
                        }
                        lastY = y;
                        last_thresh = thresh;
                    }
                }
            }
        }
    }
    return locations;
}

auto doubleRectCoords(const std::optional<RECT> &rectOpt) -> std::optional<RECT>
{
    if (rectOpt.has_value())
    {
        RECT rect = rectOpt.value();
        rect.left *= 2;
        rect.top *= 2;
        rect.right *= 2;
        rect.bottom *= 2;
        return rect;
    }
    return std::nullopt;
}

// Adjust rect using border, then clamp against the original rect itself
RECT adjustWithClamp(const RECT& rect, const RECT& border)
{
    // Apply border offsets
    RECT adjusted{
        rect.left + border.left,
        rect.top  + border.top,
        rect.left + border.left + (border.right  - border.left),
        rect.top  + border.top  + (border.bottom - border.top)
    };

    // Clamp adjusted rect against the original rect
    if (adjusted.left   < rect.left)   adjusted.left   = rect.left;
    if (adjusted.top    < rect.top)    adjusted.top    = rect.top;
    if (adjusted.right  > rect.right)  adjusted.right  = rect.right;
    if (adjusted.bottom > rect.bottom) adjusted.bottom = rect.bottom;

    // Ensure validity: if invalid, fall back to original rect
    if (adjusted.right <= adjusted.left || adjusted.bottom <= adjusted.top) {
        logError("Adjusted rect invalid, falling back to original rect");
        return rect;
    }

    return adjusted;
}


auto DetectLoop::loop(std::stop_token stopToken) -> void
{

    std::unique_ptr<ScreenCapture> screenCapture;
    logInfo("opencv", cv::getBuildInformation()); 
    auto numThreads = cv::getNumThreads();
    auto setThreads = (numThreads + 1) / 2;
    logInfo("Threads:", numThreads, "SetThreads:", setThreads);

    cv::setNumThreads(setThreads);
    auto matchScale = 1.0;
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
        int combosCount=0;
        int comboMax = comboLimit;
        int fps = 0;
        auto totalElapsed = 0.0;
        NaiveTracker l_tracker{"left: "}, d_tracker{"down: "}, u_tracker{"up:    "}, r_tracker("right:");
        while (m_loop && !stopToken.stop_requested())
        {
            auto start = CurrentMilliseconds();
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
            }

            auto screenOpt = screenCapture->grabScreen(rect);
            if(!screenOpt.has_value()){
                
                logError("Failed to grab screen");
                Sleep(5);
                continue;
            }
            cv::Mat grayScreen;
            cv::cvtColor(screenOpt.value(), grayScreen, cv::COLOR_BGR2GRAY);
            std::optional<RECT> potentialBorder = std::nullopt;
            Sleep(1);
            if (borderDetectionCount <= BORDER_MATCH_COUNT)
            {
                // Downsample the image
                cv::resize(grayScreen, grayScreen, cv::Size(), 0.5, 0.5);
                potentialBorder = detectBorder(grayScreen, MINIMUM_LINE_LENGTH);
                potentialBorder = doubleRectCoords(potentialBorder);
            }
            else
            {
                //check if captured region is correct;
                auto rW=rect.right-rect.left;
                auto rH=rect.bottom-rect.top;
                if(rW!=grayScreen.cols || rH != grayScreen.rows){
                    logError("Captured region is incorrect. Skip the frame");
                    Sleep(5);
                    continue;
                }
                matchScale = 373.0 / rW;
                logInfo("matchScale", matchScale);
                auto tt = CurrentMilliseconds();
                cv::resize(grayScreen, grayScreen, cv::Size(), matchScale, matchScale, cv::INTER_NEAREST);
                auto exitAreaY = grayScreen.rows - DETECT_AREA_HEIGHT;
                auto leftExitAreaY = exitAreaY - 6; //adjust for left lane
                auto rightExitAreaY = exitAreaY - 9; //adjust for right lane
                logInfo("Detection: exitAreaY:", exitAreaY, "leftExitAreaY:", leftExitAreaY, "rightExitAreaY:", rightExitAreaY);
                l_tracker.setExitAreaY(exitAreaY);
                d_tracker.setExitAreaY(exitAreaY);
                u_tracker.setExitAreaY(exitAreaY);
                r_tracker.setExitAreaY(exitAreaY);
                double whx = grayScreen.cols / 4.0;
                int wh0 = int(whx);
                int wh1 = int(whx * 2);
                int wh2 = int(whx * 3);
                cv::Rect matchRegion{0, 0, wh0, grayScreen.rows};
                auto lMatches = getLocationsBottomY( matchTemplateInRegion(grayScreen, leftTemplate, matchRegion), leftTemplate.rows, leftExitAreaY);
                matchRegion.x = wh0;
                auto dMatches = getLocationsBottomY( matchTemplateInRegion(grayScreen, downTemplate, matchRegion), leftTemplate.rows, exitAreaY);
                matchRegion.x = wh1;
                auto uMatches = getLocationsBottomY( matchTemplateInRegion(grayScreen, upTemplate, matchRegion), leftTemplate.rows, exitAreaY);
                matchRegion.x = wh2;
                auto rMatches = getLocationsBottomY( matchTemplateInRegion(grayScreen, rightTemplate, matchRegion), leftTemplate.rows, rightExitAreaY);
                auto cc = CurrentMilliseconds() - tt;
                logInfo("matched in ", cc, "ms", " ss: ", dc);
                if (saveForDebug)
                {
                    logInfo(
                        "Matched sizes - Left: ", lMatches.size(),
                        ", Down: ", dMatches.size(),
                        ", Up: ", uMatches.size(),
                        ", Right: ", rMatches.size());

                    auto drawLine = [&](const std::vector<int> &matches, int offsetX)
                    {
                        for (const auto &match : matches)
                        { 
                            cv::line(grayScreen, cv::Point(offsetX,match), cv::Point(offsetX+wh0,match),cv::Scalar(255, 255, 255), 2);
                        }
                    };

                    // Draw line in the bottom for each match set
                    drawLine(lMatches, 0);
                    drawLine(dMatches, wh0);
                    drawLine(uMatches, wh1);
                    drawLine(rMatches, wh2);

                    // Ensure `dc` is within valid range and save the updated gray screen
                    dc = dc % DCMAX;
                    cv::imwrite(std::to_string(dc) + ".jpg", grayScreen);
                    dc++;

                    NaiveTracker::printDetections("Left Detections", lMatches);
                    NaiveTracker::printDetections("Down Detections", dMatches);
                    NaiveTracker::printDetections("Up Detections", uMatches);
                    NaiveTracker::printDetections("Right Detections", rMatches);
                }

                NaiveTracker* trackers[] = {&l_tracker, &d_tracker, &u_tracker, &r_tracker};
                std::vector<int> matches[] = {lMatches, dMatches, uMatches, rMatches};
                int keys[] = {VK_LEFT, VK_DOWN, VK_UP, VK_RIGHT};

                for (size_t i = 0; i < 4; ++i) {
                    if (trackers[i]->updateTracker(matches[i], start)) {
                        if(combosCount < comboMax){
                            ++combosCount;
                            SimulateKeyPress(keys[i]);
                        }else{
                            logInfo("Combo limit reached. Skip the keypress");
                            combosCount = 0;
                        }
                    }
                }
                if(saveForDebug){
                    l_tracker.printLane();
                    d_tracker.printLane();
                    u_tracker.printLane();
                    r_tracker.printLane();
                } 
            }
            // Update the border if a potential border is found
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
                    rect=adjustWithClamp(rect,border); 
                    logInfo("Updated grab rectangle:", rect.left, rect.top, rect.right, rect.bottom);
                    if (saveForDebug)
                    {
                        if (potentialBorder)
                        {
                            cv::Rect cvRect(potentialBorder->left, potentialBorder->top, potentialBorder->right - potentialBorder->left, potentialBorder->bottom - potentialBorder->top);
                            cv::rectangle(screenOpt.value(), cvRect, cv::Scalar(0, 0, 255), 2);
                        }
                        cv::imwrite("screen.jpg", screenOpt.value()); 
                    }
                }
            }

            auto elapsed = CurrentMilliseconds() - start;
            totalElapsed += elapsed;
            if (totalElapsed >= 10000)
            {
                logInfo("FPS", fps / 10.0);
                totalElapsed = 0;
                fps = 0;
            }
            // Adjust CPU usage
            Sleep(static_cast<DWORD>((elapsed < 45) ? (45 - elapsed) : 10));
            fps++;
        }

        Sleep(1000);
    }
}
