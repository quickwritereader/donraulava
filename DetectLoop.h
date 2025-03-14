#pragma once
#include "utils.h"
#include "cv_utils.h"
#include <atomic>
#include "ConfigDialog.h"
#include <semaphore>
#include <thread>

/**
 * @brief DetectLoop class
 * This class will track Object in the ava dance and simulate player input
 */
class DetectLoop
{ 
 private:
    std::atomic<bool> m_loop;
    std::atomic<int> left;
    std::atomic<int> top;
    std::atomic<int> right;
    std::atomic<int> bottom;
    std::atomic<int> comboLimit;
    std::atomic<int> captureMethod;
    std::atomic<bool> saveImagesAndTracks = false;
    std::binary_semaphore sem{0};
    cv::Mat trackObject;
    uint64_t id=CurrentMilliseconds();
  public:
    DetectLoop(const cv::Mat &trackObject=cv::Mat()) : m_loop(false) {
        this->trackObject = trackObject;
        logInfo("DetectLoop constructor", id);
    }
    ~DetectLoop() {}

    auto setParameters(const RECT& screenRect, const ConfigDialog& config, bool saveDebug=FALSE) -> void {
        captureMethod = config.ScreenCaptureMethod();
        comboLimit = config.ComboThreshold();
        left = screenRect.left + config.Left();
        top = screenRect.top + config.Top();
        right = screenRect.right - config.Right();
        bottom = screenRect.bottom - config.Bottom();
        saveImagesAndTracks = saveDebug;
    }

    auto start() -> void {
        m_loop = true;
        sem.release();
    }

    auto stop()->void{
        m_loop = false;
        sem.release(); 
    }

    auto loop(std::stop_token stopToken)->void;


};
