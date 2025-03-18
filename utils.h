#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <tuple>
#include <sstream>
#include <fstream>
#include <mutex>

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

auto safeStoiDefault(const std::string &str, int defaultVal = 0) -> int;
auto safeStoiDefault(const std::wstring &text, int defaultVal=0) -> int;
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
auto GetLastErrorAsString() -> std::string;

/**
 * @brief Get the desktop screen rectangle
 * Returns a tuple containing the screen rectangle  and the DPI scale factor
 */
auto GetDesktopScreenRect() -> std::tuple<RECT, float>;

/**
 * @brief Load a Bitmap image from a resource.
 *
 * @param hInstance Handle to the instance.
 * @param resourceName Name of the resource.
 * @param resourceType Type of the resource.
 * @return Gdiplus::Bitmap* Pointer to the loaded bitmap.
 */
auto LoadBitmapFromResource(HINSTANCE hInstance, LPCSTR resourceName, LPCSTR resourceType) -> Gdiplus::Bitmap *;

template <typename T, std::size_t N>
std::ostream &operator<<(std::ostream &os, const std::array<T, N> &arr)
{
    os << "[";
    for (std::size_t i = 0; i < N; ++i)
    {
        os << arr[i];
        if (i < N - 1)
        {
            os << ", ";
        }
    }
    os << "]";
    return os;
}


class LogToFile
{
public:
    // Public method to get the instance of the singleton
    static LogToFile &getInstance()
    {
        static LogToFile instance; // Guaranteed to be thread-safe in C++11 and above
        return instance;
    }

    // Method to write a log message
    void log(const std::string &message);

    // Method to set the verbose level
    void setVerboseLevel(int verboseLevel)
    {
        verbose = verboseLevel;
    }   

    int getVerboseLevel()
    {
        return verbose;
    }


    // our opencv can't handle unicode paths
    // so we set the current working directory to the user profile
    // to avoid any issues
    static auto setCurrentWorkingDirectoryToUserProfile(std::string folderName=".donRaulTemp")->void;
    

private:
    std::ofstream logFile_;
    std::mutex mutex_;
    int verbose = 0;

    // Private constructor
    LogToFile()
    {
        logFile_.open("log.txt");
        if (!logFile_)
        {
            OutputDebugStringA("Failed to open log file\n");
        }else{
            log("--------------------------");
        }
    }

    // Private destructor
    ~LogToFile()
    {
        if (logFile_.is_open())
        {
            logFile_.close();
        }
    }

    // Delete copy constructor and assignment operator to prevent copying
    LogToFile(const LogToFile &) = delete;
    LogToFile &operator=(const LogToFile &) = delete;
}; // class LogToFile


#if !defined(OUTPUT_DBGVIEW)
#define OUTPUT_LOG(x)  \
    do                 \
    {                  \
        LogToFile::getInstance().log(x); \
    } while (0)
#else
#define OUTPUT_LOG(x)          \
    do                         \
    {                          \
        OutputDebugStringA(x); \
    } while (0)
#endif

// Function template to log error messages with space-separated variables using OutputDebugString
template <typename... Args>
void log(const std::string &messageType, Args &&...args)
{
    // Create a string stream to format the error message
    std::ostringstream oss;
    oss << "DonRaulito " << messageType << ": ";

    // Use a fold expression to append the arguments to the string stream
    ((oss << args << " "), ...);

    // Convert the formatted message to a wide string
    std::string msg = oss.str();
    // Output the message using OutputDebugString
    OUTPUT_LOG(msg.c_str());
}

template <typename... Args>
void logError(Args &&...args)
{
    log("Error", std::forward<Args>(args)...);
}

template <typename... Args>
void logInfo(Args &&...args)
{
    if(LogToFile::getInstance().getVerboseLevel() > 0){
        log("Info", std::forward<Args>(args)...);
    }
}

/**
 * @brief Get the first command line argument as an integer.
 * return 0 if no argument or invalid argument
 */
auto getFirstCommandLineArgAsInt() -> int;
