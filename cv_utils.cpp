#include "utils.h"
#include <algorithm>
#include "cv_utils.h"

auto matchTemplateInRegion(const cv::Mat &img, const cv::Mat &templ, cv::Rect region, int method, double match_threshold, bool debug) -> std::vector<cv::Point>
{
    // Adjust the region if it exceeds the image boundaries
    region.x = std::max(region.x, 0);
    region.y = std::max(region.y, 0);
    region.width = std::min(region.width, img.cols - region.x);
    region.height = std::min(region.height, img.rows - region.y);

    // Extract the region of interest (ROI) from the image
    cv::Mat img_region = img(region);

    // Determine if the ROI needs to be converted to grayscale
    cv::Mat img_gray;
    if (img_region.channels() == 3)
    {
        cv::cvtColor(img_region, img_gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        img_gray = img_region; // This does not copy the data, only references it
    }

    // Convert the template to grayscale if it's not already
    cv::Mat templ_gray;
    if (templ.channels() == 3)
    {
        cv::cvtColor(templ, templ_gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        templ_gray = templ;
    }

    // Match the template in the ROI
    cv::Mat result;
    cv::matchTemplate(img_gray, templ_gray, result, method);

    // Apply the threshold to get the matching locations
    cv::threshold(result, result, match_threshold, 1.0, cv::THRESH_TOZERO);

    // Find non-zero locations in the result
    std::vector<cv::Point> locations;
    cv::findNonZero(result, locations);

    return locations;
}
auto detectLines(const cv::Mat &img, int minLineLength) -> std::vector<cv::Vec4i>
{
    cv::Mat edges = preprocessImageForEdges(img);
    if (edges.empty())
        return {};

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, 10, minLineLength, 5);
    // cv::imwrite("gg.png", edges);

    return lines;
}

auto distance(const cv::Vec4i &line) -> double
{
    return cv::norm(cv::Point(line[0], line[1]) - cv::Point(line[2], line[3]));
}

auto preprocessImageForEdges(const cv::Mat &img) -> cv::Mat
{
    // Check if the input image is valid
    if (img.empty())
    {
        logError("Error: Empty image provided.");
        return {};
    }

    cv::Mat img_gray;
    if (img.channels() == 3)
    {
        cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        img_gray = img;
    }

    cv::Mat img_blurred;
    cv::GaussianBlur(img_gray, img_blurred, {5, 5}, 0);

    cv::Mat edges;
    cv::Canny(img_blurred, edges, 50, 200);

    return edges;
}

std::vector<cv::Vec4i> detectRectangles(const cv::Mat &img, int minWidth)
{

    cv::Mat edges = preprocessImageForEdges(img);
    if (edges.empty())
        return {};

    // Find contours in the image
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Vector to store rectangles as Vec4i
    std::vector<cv::Vec4i> rectangles;

    // Loop through the contours and approximate them to polygons
    for (size_t i = 0; i < contours.size(); i++)
    {
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contours[i], approx, 0.02 * cv::arcLength(contours[i], true), true);

        // Check if the polygon has 4 vertices (rectangle) and is convex
        if (approx.size() == 4 && cv::isContourConvex(approx))
        {
            // Calculate the width and height of the rectangle
            double width1 = cv::norm(approx[0] - approx[1]);
            double width2 = cv::norm(approx[2] - approx[3]);
            double height1 = cv::norm(approx[0] - approx[2]);
            double height2 = cv::norm(approx[1] - approx[3]);

            // Ensure the sides are horizontal and vertical and above the minimum width
            if (width1 >= minWidth && width2 >= minWidth && height1 >= minWidth && height2 >= minWidth)
            {
                // Extract the rectangle coordinates as Vec4i
                cv::Rect rect = cv::boundingRect(approx);
                rectangles.push_back(cv::Vec4i(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height));
            }
        }
    }

    return rectangles;
}

auto detectBorder(const cv::Mat &screen, int minLineLength) -> std::optional<RECT>
{
    if (screen.empty())
    {
        return std::nullopt;
    }

#if 0
    // Detect lines in the screen
    auto lines = detectLines(screen, minLineLength);
    // we intentionally put reversed rect
    if (lines.empty())
    {
        return std::nullopt;
    }

    // Initialize the border with the first line
    RECT border{std::min(lines[0][0], lines[0][2]), std::min(lines[0][1], lines[0][3]),
                std::max(lines[0][0], lines[0][2]), std::max(lines[0][1], lines[0][3])};
    bool change = true;

    for (size_t i = 1; i < lines.size(); ++i)
    {
        const auto &line = lines[i];
        LONG x1 = std::min(line[0], line[2]);
        LONG y1 = std::min(line[1], line[3]);
        LONG x2 = std::max(line[0], line[2]);
        LONG y2 = std::max(line[1], line[3]);

        if (x1 < border.left)
            border.left = x1;
        if (y1 < border.top)
            border.top = y1;
        if (x2 > border.right)
            border.right = x2;
        if (y2 > border.bottom)
            border.bottom = y2;
    }
    // our border's height and width should also greater equal than minLineLength
    if (change && (border.right - border.left >= minLineLength) && (border.bottom - border.top >= minLineLength))
    {
        //logInfo(border.left, border.top, border.right, border.bottom);
        return border;
    }
    else
        return std::nullopt;
#else
    // Detect rectangles in the screen
    auto rectangles = detectRectangles(screen, minLineLength);
    if (rectangles.empty())
    {
        return std::nullopt;
    }

    // Initialize the border with the first rectangle
    RECT border = {rectangles[0][0], rectangles[0][1], rectangles[0][2], rectangles[0][3]};
    for (const auto &rect : rectangles)
    {
        if (rect[0] < border.left)
            border.left = rect[0];
        if (rect[1] < border.top)
            border.top = rect[1];
        if (rect[2] > border.right)
            border.right = rect[2];
        if (rect[3] > border.bottom)
            border.bottom = rect[3];
    }

    // Ensure the border's height and width are greater than or equal to minLineLength
    if ((border.right - border.left >= minLineLength) && (border.bottom - border.top >= minLineLength))
    {
        return border;
    }
    else
    {
        return std::nullopt;
    }

#endif
}
