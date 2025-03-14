

#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
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

/**
 * @brief Detects lines of an image.
 * 
 * This function detects lines of an image using the Hough Line Transform.
 * 
 * @param img The source image in which to detect lines.
 * @param minLineLength
 * @return A std::vector<cv::Vec4i> containing the detected lines.
 */
auto detectLines(const cv::Mat &img, int minLineLength = 100)->std::vector<cv::Vec4i>;

/**
 * @brief Detects rectangles in an image.
 * 
 * This function detects rectangles in an image using line detection and contour approximation.
 * 
 * @param img The source image in which to detect rectangles.
 * @param minLineLength The minimum length of a line to be considered for rectangle detection. Default is 100.
 * @return A std::vector<cv::Vec4i> containing the detected rectangles.
 */
auto detectRectangles(const cv::Mat &img, int minLineLength = 100)-> std::vector<cv::Vec4i>;

/**
 * @brief Calculates the Euclidean distance of a line segment.
 * 
 * This function calculates the Euclidean distance between the endpoints of a given line segment.
 * 
 * @param line A cv::Vec4i representing the line segment, where the first two elements are the coordinates of the starting point and the last two elements are the coordinates of the ending point.
 * @return A double representing the Euclidean distance between the endpoints of the line segment.
 */
auto distance(const cv::Vec4i& line )->double;


/**
 * @brief Preprocesses an image for edge detection.
 * 
 * This function preprocesses an image to enhance edge detection by converting it to grayscale and applying Gaussian blur.
 * 
 * @param img The source image to preprocess.
 * @return A cv::Mat containing the preprocessed image.
 */
auto preprocessImageForEdges(const cv::Mat& img)->cv::Mat;

/**
 * @brief Detects the border  based on detected rectangles.
 * 
 * This function detects the border  by analyzing the detected rectangles and using the minimum LineLength.
 * 
 * @param screen The source image of the screen to analyze.
 * @param minLineLength The minimum LineLength of the detected border.
 * @return An optional RECT structure containing the detected border if found, otherwise std::nullopt.
 */
auto detectBorder(const cv::Mat &screen, int minLineLength=100) -> std::optional<RECT>;