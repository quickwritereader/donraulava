/**
 * @file WinApiScreenCapture.h
 * @brief Header file for the WinApiScreenCapture class.
 * 
 * This file contains the definition of the WinApiScreenCapture class, which is 
 * responsible for capturing screen content on Windows using the Win32 API.
 */

#pragma once

#include "ScreenCapture.h"
#include <windows.h>
#include <opencv2/opencv.hpp>

 
/**
 * @class WinApiScreenCapture
 * @brief A class for capturing screen content on Windows.
 * 
 * The WinApiScreenCapture class inherits from the ScreenCapture class and 
 * provides functionality to capture a specified region of the screen using 
 * the Win32 API and OpenCV.
 */
class WinApiScreenCapture : public ScreenCapture
{
public:
    /**
     * @brief Constructs a new WinApiScreenCapture object.
     */
    WinApiScreenCapture();

    /**
     * @brief Destroys the WinApiScreenCapture object.
     */
    ~WinApiScreenCapture();

    /**
     * @brief Captures a specified region of the screen.
     * 
     * @param region The region of the screen to capture.
     * @return A cv::Mat object containing the captured screen content.
     */
    std::optional<cv::Mat> grabScreen(RECT region) override;

private:
    HDC hScreenDC = nullptr;  ///< Handle to the screen device context.
    HDC hMemoryDC = nullptr;  ///< Handle to the memory device context..

};