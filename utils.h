#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <tuple>
#include <sstream>

/**
 * @brief Draws a window at the specified position with the given dimensions and bitmap.
 *
 * @param hWnd Handle to the window.
 * @param bitmap Reference to a pointer to a Gdiplus::Bitmap object.
 * @param x The x-coordinate of the window's position.
 * @param y The y-coordinate of the window's position.
 * @param width Reference to the width of the window.
 * @param height Reference to the height of the window.
 */
auto DrawWindow(HWND hWnd, Gdiplus::Bitmap *bitmap, LONG x, LONG y, LONG width, LONG height) -> void;
/**
 * @brief Draws a close button on the given bitmap.
 *
 * @param bitmap Pointer to a Gdiplus::Bitmap object.
 * @param width Width of the bitmap.
 * @param height Height of the bitmap.
 * @param mouseOver Boolean indicating if the mouse is over the button.
 */
auto DrawCloseBtnOnBitmap(Gdiplus::Bitmap *bitmap, LONG width, LONG height, bool mouseOver) -> void;
/**
 * @brief Checks if the current position is within the close button area.
 *
 * @param hWnd Handle to the window.
 * @param currentPos Current position of the cursor.
 * @param width Width of the window.
 * @param height Height of the window.
 * @return true if the current position is within the close button area, false otherwise.
 */
auto withinClosebtn(HWND hWnd, const POINT &currentPos, LONG width, LONG height) -> bool;
/**
 * @brief Checks and updates the close button animation based on the current cursor position.
 *
 * @param hWnd Handle to the window.
 * @param bitmap Pointer to a Gdiplus::Bitmap object.
 * @param currentPos Current position of the cursor.
 */
auto checkCloseBtnAnimation(HWND hWnd, Gdiplus::Bitmap *bitmap, const POINT &currentPos) -> void;
/**
 * @brief Applies a Gaussian blur and tint to the given bitmap.
 *
 * @param bitmap Pointer to a Gdiplus::Bitmap object.
 * @param blurRadius Radius of the Gaussian blur.
 * @param tint Color tint to apply.
 */
auto ApplyGaussianBlurTint(Gdiplus::Bitmap *bitmap, int blurRadius, Gdiplus::Color tint) -> void;
/**
 * @brief Retrieves the user profile as a string.
 *
 * @return std::string User profile.
 */
auto getUserProfile() -> std::string;
/**
 * @brief Safely converts a string to an integer with a default value.
 *
 * @param str String to convert.
 * @param def Default value to return if conversion fails.
 * @return int Converted integer or default value.
 */
auto safeStoiDefault(const std::string &str, int def = 0) -> int;
/**
 * @brief Simulates a key press event.
 *
 * @param virtualKey Virtual key code of the key to simulate.
 */
auto SimulateKeyPress(WORD virtualKey) -> void;
/**
 * @brief Retrieves the current time in milliseconds.
 *
 * @return std::uint64_t Current time in milliseconds.
 */
auto CurrentMilliseconds() -> std::uint64_t;

/**
 * @brief Retrieves the last error message as a string.
 */
auto GetLastErrorAsString()->std::string;

/**
 * @brief Get the desktop screen rectangle
 * Returns a tuple containing the screen rectangle  and the DPI scale factor
 */
auto GetDesktopScreenRect()-> std::tuple<RECT, float>;


// Function template to log error messages with space-separated variables using OutputDebugString
template <typename... Args>
void log(const std::string& messageType, Args&&... args) {
    // Create a string stream to format the error message
    std::ostringstream oss;
    oss <<"DonRaulito "<< messageType << ": ";

    // Use a fold expression to append the arguments to the string stream
    ((oss << args << " "), ...);

    // Convert the formatted message to a wide string
    std::string msg = oss.str(); 
    // Output the message using OutputDebugString
    OutputDebugStringA(msg.c_str());
}

template <typename... Args>
void logError(Args&&... args) {
    log("Error", std::forward<Args>(args)...); 
}

template <typename... Args>
void logInfo(Args&&... args) {
    log("Info", std::forward<Args>(args)...); 
}