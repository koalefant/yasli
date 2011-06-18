#include "StdAfx.h"
#include <windows.h>
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window.h"
#include "ww/Win32/Handle.h"
#include "ww/gdiplus.h"
#include "ww/PropertyTreeDrawing.h" // FIXME привести в порядок
#include "ww/Color.h"

#ifdef _MSC_VER
# define _USE_MATH_DEFINES
#endif
#include <math.h>

enum{
	OBM_CHECK = 32760
};

static HBITMAP checkBitmap = ::LoadBitmap(0, (LPCTSTR)OBM_CHECK);

namespace Win32{

int round(float v)
{
	return int(v);
}

ww::Color toColor(COLORREF c)
{
	return ww::Color((unsigned char)(c & 0x000000FF),
				   (unsigned char)((c & 0x0000FF00) >> 8),
				   (unsigned char)((c & 0x00FF0000) >> 16));
}

COLORREF blendColor(COLORREF from, COLORREF to, float factor)
{
	ww::Color c = toColor(from).interpolate(toColor(to), factor);
	return c.rgb();
}

void drawEdit3D(HDC dc, const RECT& rect, const wchar_t* text, HFONT font)
{
	RECT rt = rect;
	::FillRect(dc, &rt, GetSysColorBrush(COLOR_WINDOW));
	::DrawEdge(dc, &rt, EDGE_SUNKEN, BF_RECT);
	HFONT oldFont = (HFONT)::SelectObject(dc, font);
	::InflateRect(&rt, -3, -2);
	::DrawText(dc, text, wcslen(text), &rt, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	::SelectObject(dc, oldFont);
}

void drawEditWhiteRect(HDC dc, const RECT& rect, const wchar_t* text, HFONT font)
{
	RECT rt = rect;
	::InflateRect(&rt, -1, -1);
	::FillRect(dc, &rt, GetSysColorBrush(COLOR_WINDOW));

	HFONT oldFont = (HFONT)::SelectObject(dc, font);
	::InflateRect(&rt, -5, -2);
	rt.bottom -= 1;
	::DrawText(dc, text, wcslen(text), &rt, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	::SelectObject(dc, oldFont);
}


void drawGrayedCheck(HDC dc, const RECT& checkRect)
{
	int size = 16;
	RECT rect(checkRect);
	int offsetY = ((rect.bottom - rect.top) - size) / 2;
	int offsetX = ((rect.right - rect.left) - size) / 2;
	{
		Win32::StockSelector brush(dc, GetSysColorBrush(COLOR_BTNFACE));
		Win32::AutoSelector  pen(dc, CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW)));
		::RoundRect(dc, rect.left + offsetX, rect.top + offsetY, rect.left + offsetX + size, rect.top + offsetY + size, 3, 3);
	}
	ASSERT(checkBitmap);
	DrawState(dc, 0, 0, (LPARAM)checkBitmap, 0, rect.left + offsetX + 2, rect.top + offsetY + 1, size - 2, size - 2, DST_BITMAP | DSS_DISABLED);
}

void drawCheck(HDC dc, const RECT& checkRect, bool checked)
{
	int size = 16;
	RECT rect(checkRect);

	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, GetSysColorBrush(COLOR_WINDOW));
	HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	HPEN oldPen = (HPEN)::SelectObject(dc, pen);
	int offsetY = ((rect.bottom - rect.top) - size) / 2;
	int offsetX = ((rect.right - rect.left) - size) / 2;
	::RoundRect(dc, rect.left + offsetX, rect.top + offsetY, rect.left + offsetX + size, rect.top + offsetY + size, 3, 3);
	::SelectObject(dc, oldPen);
	::DeleteObject(pen);
	::SelectObject(dc, oldBrush);

	if(checked){
		ASSERT(checkBitmap);
		DrawState(dc, 0, 0, (LPARAM)checkBitmap, 0, rect.left + offsetX + 2, rect.top + offsetY + 1, size - 2, size - 2, DST_BITMAP);
	}
}

void drawNotCheck(Gdiplus::Graphics *gr, const Gdiplus::Rect& checkRect, bool checked)
{
	using namespace Gdiplus;
	int size = 16;

	Point center(checkRect.X + checkRect.Width / 2, checkRect.Y + checkRect.Height / 2);
	Rect rect(center.X - size / 2, center.Y - size / 2, size, size);
	if(checked){
		Color color(255, 128, 0, 0);
		SolidBrush redBrush(color);
		gr->FillEllipse(&redBrush, rect);
		rect.Inflate(-3, -3);
		gr->FillEllipse(&SolidBrush(gdiplusSysColor(COLOR_WINDOW)), gdiplusRectF(rect));
		gr->DrawEllipse(&Pen(gdiplusSysColor(COLOR_3DSHADOW)), gdiplusRectF(rect));
		int dx = round(cosf(M_PI * 0.25f) * size / 2) - 1;
		int dy = round(sinf(M_PI * 0.25f) * size / 2) - 1;
		gr->DrawLine(&Pen(color, 3), center.X - dx, center.Y + dy, center.X + dx, center.Y - dy);
	}
	else{
		gr->FillEllipse(&SolidBrush(gdiplusSysColor(COLOR_WINDOW)), gdiplusRectF(rect));
		gr->DrawEllipse(&Pen(gdiplusSysColor(COLOR_3DSHADOW)), gdiplusRectF(rect));
	}
}

void drawRadio(Gdiplus::Graphics *gr, const Gdiplus::Rect& checkRect, bool checked)
{
	using namespace Gdiplus;
	int size = 16;

	Point center(checkRect.X + checkRect.Width / 2, checkRect.Y + checkRect.Height / 2);
	Rect rect(center.X - size / 2, center.Y - size / 2, size, size);
	if(checked){
		gr->FillEllipse(&SolidBrush(gdiplusSysColor(COLOR_WINDOW)), gdiplusRectF(rect));
		gr->DrawEllipse(&Pen(gdiplusSysColor(COLOR_3DSHADOW)), gdiplusRectF(rect));
        rect.X += size / 4;
        rect.Y += size / 4;
        rect.Width -= size / 2;
        rect.Height -= size / 2;
		gr->FillEllipse(&SolidBrush(gdiplusSysColor(COLOR_BTNTEXT)), gdiplusRectF(rect));

	}
	else{
		gr->FillEllipse(&SolidBrush(gdiplusSysColor(COLOR_WINDOW)), gdiplusRectF(rect));
		gr->DrawEllipse(&Pen(gdiplusSysColor(COLOR_3DSHADOW)), gdiplusRectF(rect));
	}
}

void drawButton(HDC dc, const RECT& rect, const wchar_t* text, HFONT font)
{
	RECT rt = rect;
	::InflateRect(&rt, -1, -1);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, GetSysColorBrush(COLOR_BTNFACE));
	HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	HPEN oldPen = (HPEN)::SelectObject(dc, pen);
	::RoundRect(dc, rt.left, rt.top, rt.right, rt.bottom, 5, 5);
	::SelectObject(dc, oldPen);
	::DeleteObject(pen);
	::SelectObject(dc, oldBrush);
	HFONT oldFont = (HFONT)::SelectObject(dc, font);
	::InflateRect(&rt, -5, -2);
	rt.bottom -= 1;
	int oldBkMode = ::SetBkMode(dc, TRANSPARENT);
	::DrawText(dc, text, wcslen(text), &rt, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	::SetBkMode(dc, oldBkMode);
	::SelectObject(dc, oldFont);
}

void drawEdit(HDC dc, const RECT& rect, const wchar_t* text, HFONT font, bool pathEllipsis, bool grayBackground)
{
	RECT rt = rect;
	rt.top += 1;
	rt.left += 1;
	rt.right -= 1;
	rt.bottom -= 1;
	HBRUSH oldBrush = (HBRUSH)::SelectObject(dc, GetSysColorBrush(grayBackground ? COLOR_BTNFACE : COLOR_WINDOW));
	HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	HPEN oldPen = (HPEN)::SelectObject(dc, pen);
	::RoundRect(dc, rt.left, rt.top, rt.right, rt.bottom, 5, 5);
	::SelectObject(dc, oldPen);
	::DeleteObject(pen);
	::SelectObject(dc, oldBrush);
	HFONT oldFont = (HFONT)::SelectObject(dc, font);
	::InflateRect(&rt, -5, -1);
	rt.bottom -= 1;
	int oldBkMode = ::SetBkMode(dc, TRANSPARENT);
	::DrawText(dc, text, wcslen(text), &rt, DT_LEFT | DT_SINGLELINE | DT_VCENTER | (pathEllipsis ? DT_PATH_ELLIPSIS : DT_END_ELLIPSIS));
	::SetBkMode(dc, oldBkMode);
	::SelectObject(dc, oldFont);
}
	
void drawComboBox(HDC dc, const RECT& rect, const wchar_t* text, HFONT font)
{
	drawEdit(dc, rect, text, font);
	/*
	RECT rt = rect;
	::FillRect(dc, &rt, GetSysColorBrush(COLOR_WINDOW));
	::DrawEdge(dc, &rt, EDGE_SUNKEN, BF_RECT);
	HFONT oldFont = (HFONT)::SelectObject(dc, font);
	::InflateRect(&rt, -3, -2);
	rt.top += 1;
	rt.left += 1;
	rt.right -= GetSystemMetrics(SM_CXVSCROLL);
	::DrawText(dc, text, strlen(text), &rt, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	RECT buttonRect = rect;
	::InflateRect(&buttonRect, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));
	buttonRect.left = buttonRect.right - GetSystemMetrics(SM_CXVSCROLL);
	::DrawFrameControl(dc, &buttonRect, DFC_SCROLL, DFCS_SCROLLDOWN);
	::SelectObject(dc, oldFont);
	*/
}


void drawColorBlend(HDC dc, const ww::Rect& rect, const ww::Color& color1, const ww::Color& color2, bool alphaOnly)
{
	Win32::StockSelector pen(dc, GetStockObject(DC_PEN));
	for(int i = rect.left(); i <= rect.right (); ++i){
		float pos = float(i - rect.left()) / float(rect.width());

		ww::Color result;
		ww::Color solidColor = color1.interpolate(color2, pos);

		for(int j = 0; j < 2; ++j){
			ww::Color backColor = ((i / rect.height () + j) % 2) ?
				ww::Color(255, 255, 255) : ww::Color(0, 0, 0);

			result = backColor.interpolate(solidColor, solidColor.a);
			
			RECT r    = { i,      rect.top() + j * rect.height() / 2, 
						  i + 1, (rect.top() + j * rect.height() / 2) + (j ? 0 : (rect.height () / 2))
						};
			int y = rect.top() + j * rect.height() / 2;
			::MoveToEx(dc, i, y, 0);
			COLORREF color = (alphaOnly ? ww::Color(int(solidColor.a * 255),
											   int(solidColor.a * 255),
											   int(solidColor.a * 255), 255) : result).rgb();
			::SetDCPenColor(dc, color);
			::LineTo(dc, i, y + rect.height() / 2);
		}
	}
}

void drawVerticalBlend(HDC dc, const RECT& rect, COLORREF color1, COLORREF color2)
{
	Win32::StockSelector colorPen(dc, GetStockObject(DC_PEN));
	int height = rect.bottom - rect.top;
	for(int i = 0; i < height; ++i){
		SetDCPenColor(dc, blendColor(color1, color2, float(i) / height));
		int y = rect.top + i;
		MoveToEx(dc, rect.left, y, 0);
		LineTo(dc, rect.right - 1, y);
	}

}

void drawSlider(HDC dc, const RECT& rect, float value, bool focused)
{
	using namespace Gdiplus;
	int roundness = 5;

	{
		Graphics gr(dc);
		gr.SetSmoothingMode(SmoothingModeAntiAlias);
		gr.SetPageUnit(UnitPixel);

		LinearGradientBrush brush(gdiplusRect(rect), Color(), Color(), LinearGradientModeVertical);

		Color colors[3] = { gdiplusSysColor(COLOR_BTNFACE), gdiplusSysColor(COLOR_WINDOW), gdiplusSysColor(COLOR_WINDOW) };
		Gdiplus::REAL positions[3] = { 0.0f, 0.66f, 1.0f };
		brush.SetInterpolationColors(colors, positions, 3);

		ww::fillRoundRectangle(&gr, &brush, Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top),
			Color::Transparent, roundness);

		//ww::fillRoundRectangle(&gr, &SolidBrusH(COLOR_BTNFACE
		
		Rect filledRect( rect.left, rect.top,  round((rect.right - rect.left - 2) * clamp(value, 0.0f, 1.0f)) + 2, rect.bottom - rect.top );

		Color colors2[3] = { gdiplusSysColor(COLOR_BTNFACE), gdiplusSysColor(COLOR_BTNFACE), gdiplusSysColor(COLOR_3DSHADOW) };
		brush.SetInterpolationColors(colors2, positions, 3);

		ww::fillRoundRectangle(&gr, focused ? (Brush*)&SolidBrush(gdiplusSysColor(COLOR_HIGHLIGHT)) : &brush, filledRect, Color::Transparent, roundness);
	
		Rect handleRect( filledRect.GetRight() - 3,  filledRect.Y - 1, 4, filledRect.Height + 1);

		ww::fillRoundRectangle(&gr, &SolidBrush(Color::Transparent), Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top),
                               gdiplusSysColor(COLOR_3DSHADOW), roundness);

		gr.FillRectangle(&SolidBrush(gdiplusSysColor(COLOR_BTNFACE)), handleRect);
		gr.DrawRectangle(&Pen(gdiplusSysColor(COLOR_3DSHADOW)), handleRect);
	}
}

}
