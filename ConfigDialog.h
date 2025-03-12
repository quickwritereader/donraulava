#pragma once
#include "BaseDialog.h"
#include "utils.h"
#include <filesystem>
#include <array>
#include <fstream>
#include <ranges>
using namespace std::literals;

// ConfigDialog class
// This class is used to create a dialog box to configure the program
// It is used to set the position of the initial unadjusted capture area, the speed of the program and the combo threshold
// The configuration is saved in a file in the user profile folder
// The configuration file is when config declared and saved when the dialog is closed
// This is not thread safe. So do not make ConfigDialog a global variable
class ConfigDialog : public BaseDialog<ConfigDialog>
{

private:
	using BaseDialog::BaseDialog; // Inherit constructors from BaseDialog
	std::array<int,6> params = {430, 100, 430, 200, 25, 1080};
	std::string configFile;

	auto readConfig() -> void
	{
		// read
		std::ifstream inFile(configFile);
		if (inFile.is_open())
		{
			std::string res;
			if (std::getline(inFile, res))
			{
				auto split_view = res | std::ranges::views::split(':');
				//split create a nested range
				//and explicitely convert it to a string
				//this is better when there can be corrupted data
				int i=0;
				for (auto range : split_view)
				{
					std::string str(range.begin(), range.end());
					//MessageBox(NULL, (str).c_str(), "Info", MB_OK | MB_ICONINFORMATION);
					params[i]=safeStoiDefault(str);
					i++;
					//break if we reach the end of the array
					if(i>=params.size()) break;
				}
				
			}
			inFile.close();
		}
	}
	auto
	save() -> void
	{
		try
		{

			// Create and write to file
			std::ofstream outFile(configFile);
			if (outFile.is_open())
			{
				for (auto param : params)
				{
					outFile << std::to_string(param) << ":";
				}
				outFile.close();
			}
		}
		catch (...)
		{
			MessageBox(NULL, "Error saving config file", "Error", MB_OK | MB_ICONERROR);
		}
	}

	auto fillParamFromEdit() -> void
	{
		int controlId[6] = {IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6};
        auto view = controlId | std::views::transform([this](auto Id)
                                                      { return safeStoiDefault(GetText(Id)); });
        std::ranges::copy(view, params.begin());
    }



    virtual auto onInit()->void{
		int controlId[6] = {IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6}; 
		for (int i=0;i<params.size();i++){
			SetText(controlId[i], std::to_string(params[i]));
		}
	}

	virtual auto
	handleCommand(UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
	{

		switch (LOWORD(wParam))
		{ // LOWORD(wParam) is the control ID
		case IDOK:
			if (LOWORD(wParam) == IDOK)
			{
				fillParamFromEdit();
				save();
			}
		//fall through to IDCANCEL
		case IDCANCEL:
			endDialog(wParam);
			return TRUE;
		case IDC_RESET:
			params = {430, 100, 430, 200, 25, 1080};
			onInit();
			return TRUE;
		default:
			return FALSE;

		}
	}

public:


	ConfigDialog(HINSTANCE inst, HWND parent) : ConfigDialog(inst, parent, IDD_DIALOG1)
	{
		// Get user config file
		configFile = getUserProfile() + "\\donraul.config";
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

	//Getters
	//This should be used only in the main thread
    //WARNING: Do not use this outside of UI Thread

	auto Left() const -> int 
	{
		return params[0];
	}
	auto Top() const -> int 
	{
		return params[1];
	}
	auto Right() const -> int 
	{
		return params[2];
	}
	auto Bottom() const -> int 
	{
		return params[3];
	}

	auto Speed() const -> int 
	{
		return params[4];
	}

	auto ComboThreshold() const -> int 
	{
		return params[5];
	}
};
