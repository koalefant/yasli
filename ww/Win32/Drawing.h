#pragma once

#include "ww/Win32/Types.h"
#include "ww/Rect.h"

class KeysColor;
struct Color4f;
struct Color4c;

namespace Gdiplus{
	class Graphics;
	class Rect;
}

namespace Win32{
	using namespace yasli;

	COLORREF toColorRef(const Color4f& color);
	Color4c toColor4c(COLORREF c);
	COLORREF blendColor(COLORREF from, COLORREF to, float factor);
	// void drawGradient(HDC dc, const RECT& rect, KeysColor& gradient, bool alphaOnly = false);
	void drawColorBlend(HDC dc, const ww::Rect& rect, const Color4f& color1, const Color4f& color2, bool alphaOnly = false);

	void drawVerticalBlend(HDC dc, const RECT& rect, COLORREF color1, COLORREF color2);

	void drawGrayedCheck(HDC dc, const RECT& iconRect);
    void drawRadio(Gdiplus::Graphics *gr, const Gdiplus::Rect& checkRect, bool checked);
	void drawNotCheck(Gdiplus::Graphics* gr, const Gdiplus::Rect& rect, bool checked);
	void drawEdit(HDC dc, const RECT& rect, const wchar_t* text, HFONT font, bool pathEllipsis = false, bool grayBackground = false);
	void drawButton(HDC dc, const RECT& rect, const wchar_t* text, HFONT font);
	void drawComboBox(HDC dc, const RECT& rect, const wchar_t* text, HFONT font);
	void drawSlider(HDC dc, const RECT& rect, float value, bool focused);
}

