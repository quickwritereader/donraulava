#pragma once
#include "utils.h"
#include "cv_utils.h"
#include <atomic>
#include "ConfigDialog.h"
#include <semaphore>
#include <thread>

/**
 * @brief DetectLoop class
 * This class will track an object in the ava dance and simulate player input.
 */
class DetectLoop
{
private:
    std::atomic<bool> m_loop; ///< Indicates if the loop is running
    std::atomic<int> left; ///< Left boundary for tracking
    std::atomic<int> top; ///< Top boundary for tracking
    std::atomic<int> right; ///< Right boundary for tracking
    std::atomic<int> bottom; ///< Bottom boundary for tracking
    std::atomic<int> comboLimit; ///< Combo limit for tracking
    std::atomic<int> captureMethod; ///< Capture method for tracking
    std::atomic<bool> saveImagesAndTracks = false; ///< Flag to save images and tracks
    std::binary_semaphore sem{0}; ///< Semaphore for synchronization
    cv::Mat trackObject; ///< Object to be tracked
    uint64_t id = CurrentMilliseconds(); ///< Unique ID for the instance
    std::jthread detectThread; ///< Thread for detection loop

public:
    /**
     * @brief Construct a new DetectLoop object
     * 
     * @param trackObject Object to be tracked (default is empty cv::Mat)
     */
    DetectLoop(const cv::Mat &trackObject = cv::Mat()) : m_loop(false)
    {
        this->trackObject = trackObject;
    }

    /**
     * @brief Set the object to be tracked
     * 
     * @param trackObject Object to be tracked
     * @return true if the object was set successfully
     * @return false if the detection thread is already running
     */
    auto setTrackObj(const cv::Mat &trackObject) -> bool
    {
        if (!detectThread.joinable())
        {
            this->trackObject = trackObject;
            return true;
        }
        return false;
    }

    /**
     * @brief Set the parameters for tracking
     * 
     * @param screenRect Screen rectangle for tracking boundaries
     * @param config Configuration dialog for settings
     * @param saveDebug Flag to save debug images and tracks (default is false)
     */
    auto setParameters(const RECT& screenRect, const ConfigDialog& config, bool saveDebug = false) -> void
    {
        // Capture method and thresholds
        captureMethod = config.ScreenCaptureMethod();
        comboLimit    = config.ComboThreshold();

        // Apply offsets
        left   = screenRect.left   + config.Left();
        top    = screenRect.top    + config.Top();
        right  = screenRect.right  - config.Right();
        bottom = screenRect.bottom - config.Bottom();

        saveImagesAndTracks = saveDebug;

        // Validate region
        if (left >= right || top >= bottom) {
            logError("Invalid capture region after applying config offsets. Falling back to defaults.");

            // Fallback to defaults (use original screenRect without offsets)
            left   = screenRect.left;
            top    = screenRect.top;
            right  = screenRect.right;
            bottom = screenRect.bottom;
        }
    }

    /**
     * @brief Start the detection loop
     */
    auto start() -> void
    {
        // Check if we're inside the same thread as the loop
        if (std::this_thread::get_id() == detectThread.get_id())
        {
            logError("start called from within the loop. Ignored.");
            return; // Prevent start() from being called inside the loop
        }
        if (!detectThread.joinable())
        {
            detectThread = std::jthread([this](std::stop_token stopToken) mutable
                                        { this->loop(stopToken); });
        }
    }

    /**
     * @brief Resume the detection loop
     */
    auto resume() -> void
    {
        // Check if we're inside the same thread as the loop
        if (std::this_thread::get_id() == detectThread.get_id())
        {
            logError("resume called from within the loop. Ignored.");
            return; // Prevent resume() from being called inside the loop
        }
        m_loop = true;
        sem.release();
    }

    /**
     * @brief Pause the detection loop
     */
    auto pause() -> void
    {
        // Check if we're inside the same thread as the loop
        if (std::this_thread::get_id() == detectThread.get_id())
        {
            logError("pause called from within the loop. Ignored.");
            return; // Prevent pause() from being called inside the loop
        }
        m_loop = false;
        sem.release();
    }

    /**
     * @brief Stop the detection loop
     */
    auto stop() -> void
    {
        // Check if we're inside the same thread as the loop
        if (std::this_thread::get_id() == detectThread.get_id())
        {
            logError("Stop called from within the loop. Ignored.");
            return; // Prevent stop() from being called inside the loop
        }
        // Ensure proper cleanup if thread was ever started
        if (detectThread.joinable())
        {
            // notify our loop for end
            pause();
            detectThread.request_stop();
            detectThread.join();
        }
    }

    /**
     * @brief The main loop for detection
     * 
     * @param stopToken Token to stop the loop
     */
    auto loop(std::stop_token stopToken) -> void;

    /**
     * @brief Destroy the DetectLoop object
     */
    ~DetectLoop()
    {
        stop();
    }
};
