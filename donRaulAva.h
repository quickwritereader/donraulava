// donRaulAva.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

#include "ConfigDialog.h"
#include "utils.h"
#pragma comment(lib, "gdiplus.lib")
#define WM_CONFIG_USER  (WM_USER + 100)
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
auto checkCloseBtnAnimation(HWND hWnd, Gdiplus::Bitmap *bitmap, const POINT &currentPos) -> void;