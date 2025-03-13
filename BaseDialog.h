#pragma once
#include "utils.h"
#include "resource.h"

/**
 * @class BaseDialog
 * @brief A base class template for creating dialogs in a Windows application.
 * 
 * This class provides a base implementation for creating and managing dialogs.
 * Derived classes should implement the onInit and handleCommand methods to handle
 * to achieve static polymorphism. 
 * @tparam Derived The derived class type.
 */
template <typename Derived>
class BaseDialog
{
public:
	/**
	 * @brief Construct a new Base Dialog object.
	 * 
	 * @param hInstance Handle to the instance.
	 * @param hWndParent Handle to the parent window.
	 * @param templateId Resource ID of the dialog template.
	 */
	BaseDialog(HINSTANCE hInstance, HWND hWndParent, int templateId);

	/**
	 * @brief Open the dialog.
	 */
	auto open() -> void;

	/**
	 * @brief Set text in a control (edit control, static control, etc.).
	 * 
	 * @param controlID The ID of the control.
	 * @param text The text to set in the control.
	 * @return true if the text was set successfully, false otherwise.
	 */
	bool SetText(int controlID, const std::string &text);

	/**
	 * @brief Get text from a control (edit control, static control, etc.).
	 * 
	 * @param controlID The ID of the control.
	 * @return The text from the control.
	 */
	std::string GetText(int controlID) const;

	/**
	 * @brief End the dialog.
	 * 
	 * @param wParam Additional message-specific information.
	 */
	void endDialog(WPARAM wParam);

	/**
	 * @brief Handle command messages.
	 * 
	 * @param message The message.
	 * @param wParam Additional message-specific information.
	 * @param lParam Additional message-specific information.
	 * @return The result of the message processing.
	 */
	auto handleCommand(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;

	/**
	 * @brief Initialize the dialog.
	 */
	auto onInit() -> void;

private:
	HWND self = nullptr; ///< Handle to the dialog.
	HINSTANCE inst; ///< Handle to the instance.
	HWND parent; ///< Handle to the parent window.
	int tmplId; ///< Resource ID of the dialog template.

	/**
	 * @brief Dialog procedure.
	 * 
	 * @param message The message.
	 * @param wParam Additional message-specific information.
	 * @param lParam Additional message-specific information.
	 * @return The result of the message processing.
	 */
	auto proc(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;

	/**
	 * @brief Static dialog procedure.
	 * 
	 * @param hDlg Handle to the dialog.
	 * @param message The message.
	 * @param wParam Additional message-specific information.
	 * @param lParam Additional message-specific information.
	 * @return The result of the message processing.
	 */
	static LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

template <typename Derived>
BaseDialog<Derived>::BaseDialog(HINSTANCE hInstance, HWND hWndParent, int templateId)
	: inst{hInstance}, parent{hWndParent}, tmplId{templateId}
{
}

template <typename Derived>
auto BaseDialog<Derived>::open() -> void
{
	if (!self)
	{
		auto ret = CreateDialogParam(inst, MAKEINTRESOURCE(tmplId), parent, BaseDialog::DialogProc, reinterpret_cast<LPARAM>(this));
		if (!ret)
		{
			MessageBox(parent, GetLastErrorAsString().c_str(), "error", MB_OK);
		}
	}
}

template <typename Derived>
bool BaseDialog<Derived>::SetText(int controlID, const std::string &text)
{
	HWND hControl = GetDlgItem(self, controlID);
	if (!hControl)
	{
		logError( GetLastErrorAsString());
		return false;
	}
	return SetDlgItemTextA(self, controlID, text.c_str()) != 0;
}

template <typename Derived>
std::string BaseDialog<Derived>::GetText(int controlID) const
{
	HWND hControl = GetDlgItem(self, controlID);
	if (!hControl)
	{
		logError( GetLastErrorAsString());
		return "";
	}

	int length = GetWindowTextLengthA(hControl);
	if (length == 0)
	{
		return "";
	}

	std::string text;
	text.resize(length + 1);
	GetWindowTextA(hControl, text.data(), length + 1);
	return text;
}

template <typename Derived>
auto BaseDialog<Derived>::onInit() -> void
{ 
	//static polymorphism using CRTP
	static_cast<Derived *>(this)->onInit();
}

template <typename Derived>
auto BaseDialog<Derived>::handleCommand(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
 //static polymorphism using CRTP
	return 	static_cast<Derived *>(this)->handleCommand(message, wParam, lParam);
}

template <typename Derived>
void BaseDialog<Derived>::endDialog(WPARAM wParam)
{
	::EndDialog(self, LOWORD(wParam));
	self = nullptr;
}

template <typename Derived>
auto BaseDialog<Derived>::proc(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	switch (message)
	{
	case WM_INITDIALOG:
		onInit();
		return (LRESULT)TRUE;
	case WM_COMMAND:
		return handleCommand(message, wParam, lParam);
	}
	return (LRESULT)FALSE;
}

template <typename Derived>
LRESULT CALLBACK BaseDialog<Derived>::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		auto pDlg = reinterpret_cast<BaseDialog *>(lParam);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlg));
		pDlg->self = hDlg;
		return pDlg->proc(message, wParam, lParam);
	}
	default:
		auto dlg = reinterpret_cast<BaseDialog *>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
		return dlg->proc(message, wParam, lParam);
	}
	return (INT_PTR)FALSE;
}
