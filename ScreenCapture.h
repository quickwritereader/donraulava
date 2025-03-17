/**
 * @file ScreenCapture.h
 * @brief Defines the ScreenCapture class for capturing screen regions.
 */

#pragma once
#include "utils.h"
#include <opencv2/opencv.hpp>

/**
 * @class ScreenCapture
 * @brief Abstract base class for screen capturing functionality.
 */
class ScreenCapture {
public:
    /**
     * @brief Virtual destructor for ScreenCapture.
     */
    virtual ~ScreenCapture() = default;

    /**
     * @brief Captures a specified region of the screen.
     * @param region The RECT structure defining the region of the screen to capture.
     * @return A cv::Mat object containing the captured screen region.
     */
    virtual std::optional<cv::Mat> grabScreen(RECT region) = 0;
};
