#pragma once
#include "BaseDialog.h"
#include <array>


// ConfigDialog class
// This class is used to create a dialog box to configure the program
// It is used to set the position of the initial unadjusted capture area, the speed of the program and the combo threshold
// The configuration is saved in a file in the user profile folder
// The configuration file is when config declared and saved when the dialog is closed
// This is not thread safe. So do not make ConfigDialog a global variable
class ConfigDialog : public BaseDialog<ConfigDialog>
{
private:
	using BaseDialog::BaseDialog;
	std::array<int, 6> params = {430, 100, 430, 200, 25, 1080};
	std::string configFile;

	/**
	 * @brief Reads the configuration from the configuration file.
	 */
	auto readConfig() -> void;

	/**
	 * @brief Saves the current configuration to the configuration file.
	 */
	auto save() -> void;

	/**
	 * @brief Fills the parameters from the edit controls in the dialog.
	 */
	auto fillParamFromEdit() -> void;
public:
    //Note: onInit and handleCommand should be public in the derived class
	//Note: to make static polymorphism work. or we should make BaseDialog a friend class
	/**
	 * @brief Initializes the dialog box.
	 */
	auto onInit() -> void;

	/**
	 * @brief Handles command messages sent to the dialog box.
	 *
	 * @param message The message identifier.
	 * @param wParam Additional message information.
	 * @param lParam Additional message information.
	 * @return The result of the message processing.
	 */
	auto handleCommand(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;


	/**
	 * @brief Constructs a ConfigDialog object.
	 *
	 * @param inst The instance handle.
	 * @param parent The handle to the parent window.
	 */
	ConfigDialog(HINSTANCE inst, HWND parent);

	/**
	 * @brief Gets the left position of the capture area.
	 *
	 * @return The left position.
	 */
	auto Left() const -> int;

	/**
	 * @brief Gets the top position of the capture area.
	 *
	 * @return The top position.
	 */
	auto Top() const -> int;

	/**
	 * @brief Gets the right position of the capture area.
	 *
	 * @return The right position.
	 */
	auto Right() const -> int;

	/**
	 * @brief Gets the bottom position of the capture area.
	 *
	 * @return The bottom position.
	 */
	auto Bottom() const -> int;

	/**
	 * @brief Gets the speed of the program.
	 *
	 * @return The speed.
	 */
	auto Speed() const -> int;

	/**
	 * @brief Gets the combo threshold.
	 *
	 * @return The combo threshold.
	 */
	auto ComboThreshold() const -> int;
};
