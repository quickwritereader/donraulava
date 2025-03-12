#include "cv_utility.h"

auto matchTemplateInRegion(const cv::Mat &img, const cv::Mat &templ, cv::Rect region, int method = cv::TM_CCOEFF_NORMED, double match_threshold = 0.8, bool debug = false) -> std::vector<cv::Point>{
    // Adjust the region if it exceeds the image boundaries
    region.x = std::max(region.x, 0);
    region.y = std::max(region.y, 0);
    region.width = std::min(region.width, img.cols - region.x);
    region.height = std::min(region.height, img.rows - region.y);

    // Extract the region of interest (ROI) from the image
    cv::Mat img_region = img(region);

    // Determine if the ROI needs to be converted to grayscale
    cv::Mat img_gray;
    if (img_region.channels() == 3) {
        cv::cvtColor(img_region, img_gray, cv::COLOR_BGR2GRAY);
    } else {
        img_gray = img_region;  // This does not copy the data, only references it
    }

    // Convert the template to grayscale if it's not already
    cv::Mat templ_gray;
    if (templ.channels() == 3) {
        cv::cvtColor(templ, templ_gray, cv::COLOR_BGR2GRAY);
    } else {
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

    // If debug is enabled, draw rectangles around the matching regions
    if (debug) {
        cv::Mat debug_img = img.clone();
        for (const auto &pt : locations) {
            cv::Point pt2(pt.x + region.x, pt.y + region.y);
            cv::rectangle(debug_img, cv::Rect(pt2.x, pt2.y, templ.cols, templ.rows), cv::Scalar(0, 0, 255), 2);
        }
        //save the image
        cv::imwrite("debug_img.jpg", debug_img);
        
    }

    return locations;
}
