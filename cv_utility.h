
#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief Matches a template within a specified region of an image.
 * 
 * This function searches for the best match of a template image within a specified region of a source image using a specified matching method.
 * 
 * @param img The source image in which to search for the template.
 * @param templ The template image to search for within the source image.
 * @param region The region of the source image to search within.
 * @param method The comparison method to use for template matching. Default is cv::TM_CCOEFF_NORMED.
 * @param match_threshold The threshold value for considering a match. Default is 0.8.
 * @param debug If true, enables debug mode which may provide additional output for debugging purposes. Default is false.
 * @return A std::vector<cv::Point> containing the result of the template matching.
 */
auto matchTemplateInRegion(const cv::Mat &img, const cv::Mat &templ, cv::Rect region, int method = cv::TM_CCOEFF_NORMED, double match_threshold = 0.8, bool debug = false)->std::vector<cv::Point>; ;


 