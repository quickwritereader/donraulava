/**
 * @file DesktopDuplicateCapture.h
 * @brief Header file for DesktopDuplicationCapture class using Desktop Duplication API.
 */

#pragma once

#include "ScreenCapture.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace Microsoft::WRL;

/**
 * @class DesktopDuplicationCapture
 * @brief A class for capturing the desktop screen using the Desktop Duplication API.
 */
class DesktopDuplicationCapture : public ScreenCapture {
public:
    /**
     * @brief Constructor for DesktopDuplicationCapture.
     * @param OutputNumber The output number to capture from (default is 0).
     */
    DesktopDuplicationCapture(UINT OutputNumber = 0);

    /**
     * @brief Destructor for DesktopDuplicationCapture.
     */
    ~DesktopDuplicationCapture();

    /**
     * @brief Captures a region of the screen.
     * @param region The region of the screen to capture.
     * @return A cv::Mat object containing the captured screen region.
     */
    cv::Mat grabScreen(RECT region) override;

private:
    /**
     * @brief Initializes the Desktop Duplication API.
     * @param OutputNumber The output number to capture from.
     */
    void Initialize(UINT OutputNumber);

    /**
     * @brief Closes the Desktop Duplication API and releases resources.
     */
    void Close();

    /**
     * @brief Captures the next frame from the desktop.
     * @param region The region of the screen to capture.
     * @return True if the capture was successful, false otherwise.
     */
    bool CaptureNext(const RECT& region);

    IDXGIOutputDuplication* DeskDupl = nullptr; ///< Pointer to the IDXGIOutputDuplication interface.
    ID3D11Device* D3DDevice = nullptr; ///< Pointer to the ID3D11Device interface.
    ID3D11DeviceContext* D3DDeviceContext = nullptr; ///< Pointer to the ID3D11DeviceContext interface.
    DXGI_OUTPUT_DESC OutputDesc; ///< Description of the output display.
    bool HaveFrameLock = false; ///< Indicates if the frame is locked.

    /**
     * @struct FrameData
     * @brief Structure to hold the latest frame data.
     */
    struct FrameData {
        int Width = 0; ///< Width of the captured frame.
        int Height = 0; ///< Height of the captured frame.
        std::vector<uint8_t> Buf; ///< Buffer to hold the frame data.
    } Latest; ///< Latest captured frame data.
};
