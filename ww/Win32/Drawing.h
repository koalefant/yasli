#pragma once

#include "ww/Win32/Types.h"
#include "ww/Rect.h"
#include "XMath/Colors.h"

class KeysColor;

namespace Gdiplus{
	class Graphics;
	class Rect;
}

namespace Win32{
	using namespace yasli;

	inline COLORREF toColorRef(const Color4f& color){
		return COLORREF(((unsigned char)(round(color.r * 255.0f))|
			     ((unsigned short)((unsigned char)(round(color.g * 255.0f)))<<8))|
				(((unsigned int)(unsigned char)(round(color.b * 255.0f)))<<16));
	}
	inline Color4c toColor4c(COLORREF c){
		return Color4c((unsigned char)(c & 0x000000FF),
                       (unsigned char)((c & 0x0000FF00) >> 8),
                       (unsigned char)((c & 0x00FF0000) >> 16));
	}
	inline COLORREF blendColor(COLORREF from, COLORREF to, float factor)
	{
		Color4c c;
		c.interpolate(toColor4c(from), toColor4c(to), factor);
		return c.rgb();
	}
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

