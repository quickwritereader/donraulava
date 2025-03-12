#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
auto DrawWindow(HWND hWnd, LONG x, LONG y, LONG &width, LONG &height, Gdiplus::Bitmap *&bitmap) -> void;
auto DrawCloseBtn(Gdiplus::Bitmap *bitmap, LONG width, LONG height, bool mouseOver) -> void;
auto withinClosebtn(HWND hWnd, const POINT &currentPos, LONG width, LONG height) -> bool;
auto checkCloseBtnAnimation(HWND hWnd, Gdiplus::Bitmap *bitmap, const POINT &currentPos) -> void;
auto ApplyGaussianBlurTint(Gdiplus::Bitmap *bitmap, int blurRadius, Gdiplus::Color tint) ->void;
auto getUserProfile() -> std::string;
auto safeStoiDefault(const std::string &str, int def = 0) -> int;
auto SimulateKeyPress(WORD virtualKey)->void;