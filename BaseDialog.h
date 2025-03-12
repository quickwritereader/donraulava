#pragma once
#include <windows.h>
#include "resource.h"
#include <string>

std::string GetLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
	{
		return std::string(); // No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);
	LocalFree(messageBuffer);
	return message;
}

template <typename Derived>
class BaseDialog
{
public:
	BaseDialog(HINSTANCE hInstance, HWND hWndParent, int templateId) : inst{hInstance}, parent{hWndParent}, tmplId{templateId}
	{
	}

	auto open() -> void
	{
		if (!self)
		{

			auto ret=CreateDialogParam(inst, MAKEINTRESOURCE(tmplId), parent, BaseDialog::DialogProc, reinterpret_cast<LPARAM>(this));

			if (!ret)
			{
				MessageBox(parent, GetLastErrorAsString().c_str(), "error", MB_OK);
			}
		}
	}



	// Function to set text in a control (edit control, static control, etc.)
	bool SetText(int controlID, const std::string &text)
	{
		// Get a handle to the control
		
		HWND hControl = GetDlgItem(self, controlID);
		if (!hControl)
		{
			MessageBox(parent, GetLastErrorAsString().c_str(), "error", MB_OK);
			// Return false if the control handle is invalid
			return false;
		}
		return SetDlgItemTextA(self, controlID, text.c_str()) != 0;
	}

	// Function to get text from a control (edit control, static control, etc.)
	std::string GetText(int controlID) const
	{
		// Get a handle to the control
		HWND hControl = GetDlgItem(self, controlID);
		if (!hControl)
		{
			MessageBox(parent, GetLastErrorAsString().c_str(), "error", MB_OK);
			// Return an empty string if the control handle is invalid
			return "";
		}

		// Get the length of the text in the control
		int length = GetWindowTextLengthA(hControl);
		if (length == 0)
		{
			// Return an empty string if there's no text
			return "";
		}

		std::string text;
		text.resize(length + 1);

		// Get the text from the control
		GetWindowTextA(hControl, text.data(), length + 1); 
		return text;
	}

	void endDialog(WPARAM wParam)
	{
		::EndDialog(self, LOWORD(wParam));
		self = nullptr;
	}

	virtual auto handleCommand(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT = 0;
	virtual auto onInit() -> void = 0;



private:
	HWND self = nullptr;
	HINSTANCE inst;
	HWND parent;
	int tmplId;

	auto proc(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
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

	static LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// UNREFERENCED_PARAMETER(lParam);

		switch (message)
		{

		case WM_INITDIALOG:
		{
			auto pDlg = reinterpret_cast<Derived *>(lParam);
			SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlg));
			pDlg->self = hDlg; //set the handle to the 
			return pDlg->proc(message, wParam, lParam);
		}
		break;
		default:
			// redirect to class implementation
			auto dlg = reinterpret_cast<Derived *>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
			return dlg->proc(message, wParam, lParam);
		}
		return (INT_PTR)FALSE;
	}
};
