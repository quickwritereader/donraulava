#include "ConfigDialog.h"
#include <filesystem>
#include <fstream>
#include <ranges>

using namespace std::literals;

//PRIVATE declarations
auto ConfigDialog::readConfig() -> void
{
	// read
	std::ifstream inFile(configFile);
	if (inFile.is_open())
	{
		std::string res;
		if (std::getline(inFile, res))
		{
			auto split_view = res | std::ranges::views::split(':');
			// split create a nested range
			// and explicitly convert it to a string
			// this is better when there can be corrupted data
			int i = 0;
			logInfo("Read config: ", res);
			for (auto range : split_view)
			{
				std::string str(range.begin(), range.end());
				// logError( (str).c_str(), "Info", MB_OK | MB_ICONINFORMATION);
				params[i] = safeStoiDefault(str);
				
				logInfo("Read config: param[",i, "] = ", str);
				i++;
				// break if we reach the end of the array
				if (i >= params.size()) break;
			}
		}
		inFile.close();
	}
}

auto ConfigDialog::save() -> void
{
	try
	{
		// Create and write to file
		std::ofstream outFile(configFile);
		if (outFile.is_open())
		{
			logInfo("Save config: ", params);
			for (auto param : params)
			{
				outFile << std::to_string(param) << ":";
			}
			outFile.close();
		}
	}
	catch (...)
	{
		logError( "Error saving config file");
	}
}

auto ConfigDialog::fillParamFromEdit() -> void
{
	int controlId[6] = {IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6};
	auto view = controlId | std::views::transform([this](auto Id)
												  { return safeStoiDefault(GetText(Id)); });
	std::ranges::copy(view, params.begin());

	// Get the selected item from the combo box
	auto hComboBox = GetDlgItem(IDC_COMBO1);
	if(hComboBox){
	    auto x = SendMessageA(hComboBox, CB_GETCURSEL, 0, 0);
		logInfo("Combo box value: ", x);
		if (x>=0 and x<=1){
			params[6] = (int)x ;
		}
	}

}

auto ConfigDialog::onInit() -> void
{
	int controlId[6] = {IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6};
	for (int i = 0; i < 6; i++)
	{
		SetText(controlId[i], std::to_string(params[i]));
	}
	auto hComboBox = GetDlgItem(IDC_COMBO1);
	if(hComboBox){
		logInfo("Combo box setup");
        SendMessage(hComboBox, CB_RESETCONTENT, 0, 0); // Clear any existing items

        // Add items to the combo box and set selected index
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)"DDAPI");
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)"WIN32");
		SendMessageA(hComboBox, CB_SETCURSEL, params[6], 0);
	}

}

auto ConfigDialog::handleCommand(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	switch (LOWORD(wParam))
	{ // LOWORD(wParam) is the control ID
	case IDOK:
		if (LOWORD(wParam) == IDOK)
		{
			fillParamFromEdit();
			save();
		}
	// NOTE fall through to IDCANCEL
	case IDCANCEL:
		endDialog(wParam);
		return TRUE;
	case IDC_RESET:
		params = {430, 100, 430, 100, 25, 1080, 0};
		onInit();
		return TRUE;
	default:
		return FALSE;
	}
}

//PUBLIC declarations
ConfigDialog::ConfigDialog(HINSTANCE inst, HWND parent) : ConfigDialog(inst, parent, IDD_DIALOG1)
{
	// Get user config file
	configFile = "donraul.config";
	// Check if file exists
	if (std::filesystem::exists(configFile))
	{
		readConfig();
	}
	else
	{
		save();
	}
}

auto ConfigDialog::Left() const -> int
{
	return params[0];
}

auto ConfigDialog::Top() const -> int
{
	return params[1];
}

auto ConfigDialog::Right() const -> int
{
	return params[2];
}

auto ConfigDialog::Bottom() const -> int
{
	return params[3];
}

auto ConfigDialog::Speed() const -> int
{
	return params[4];
}

auto ConfigDialog::ComboThreshold() const -> int
{
	return params[5];
}

auto ConfigDialog::ScreenCaptureMethod() const -> int {
	return params[6];
}