#include "stdafx.h"
#include "PropertyDrawContext.h"
#include <windows.h>
#include <memory>
#include "gdiplus.h"
#include "yasli/Assert.h"
#include "Win32/Drawing.h"
#include "PropertyTree.h"
#include "Color.h"

using namespace Gdiplus;

namespace ww{

static int drawingUsers = 0;
static ULONG_PTR drawingToken;
enum{
	OBM_CHECK = 32760
};

static HBITMAP checkBitmap = ::LoadBitmap(0, (LPCTSTR)OBM_CHECK);
static std::auto_ptr<Gdiplus::Font> defaultFont;
static std::auto_ptr<Gdiplus::Font> defaultBoldFont;

Gdiplus::Font* propertyTreeDefaultFont()
{
	return defaultFont.get();
}

Gdiplus::Font* propertyTreeDefaultBoldFont()
{
	return defaultBoldFont.get();
}

void drawingInit()
{
	Gdiplus::GdiplusStartupInput startup;
	startup.DebugEventCallback = 0;
	startup.GdiplusVersion = 1;
	startup.SuppressBackgroundThread = FALSE;

	if(drawingUsers == 0){
		ESCAPE(GdiplusStartup(&drawingToken, &startup, 0) == Ok, return);

		NONCLIENTMETRICS nonClientMetrics;
		nonClientMetrics.cbSize = sizeof(nonClientMetrics);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), (PVOID)&nonClientMetrics, 0);
        defaultFont.reset(new Gdiplus::Font(GetDC(GetDesktopWindow()), &nonClientMetrics.lfMessageFont));
		
		nonClientMetrics.lfMessageFont.lfWeight = FW_BOLD;
        defaultBoldFont.reset(new Gdiplus::Font(GetDC(GetDesktopWindow()), &nonClientMetrics.lfMessageFont));
    }


	++drawingUsers;
}

void drawingFinish()
{
	ESCAPE(drawingUsers >= 0, return);
	--drawingUsers;
	if(drawingUsers == 0)
	{
		defaultFont.reset();
		defaultBoldFont.reset();
		GdiplusShutdown(drawingToken);
	}
}

void createRoundRectanglePath(GraphicsPath* path, const Gdiplus::Rect &_rect, int dia)
{
	using Gdiplus::Rect;
    Rect r = _rect;
    if(dia > r.Width)
        dia = r.Width;
    if(dia > r.Height)  
        dia = r.Height;

    Rect corner(r.X, r.Y, dia, dia);
    path->Reset();

    path->AddArc(corner, 180, 90);    

    if(dia == 20) {
        corner.Width += 1; 
        corner.Height += 1; 
        r.Width -=1; r.Height -= 1;
    }

    corner.X += (r.Width - dia - 1);
    path->AddArc(corner, 270, 90);    
    
    corner.Y += (r.Height - dia - 1);
    path->AddArc(corner,   0, 90);    
    
    corner.X -= (r.Width - dia - 1);
    path->AddArc(corner,  90, 90);
    path->CloseFigure();
}

void drawRoundRectangle(Gdiplus::Graphics* graphics, const Gdiplus::Rect &_r, unsigned int color, int radius, int width)
{
	Gdiplus::Rect r = _r;
    int dia = 2*radius;

    int oldPageUnit = graphics->SetPageUnit(UnitPixel);

	Pen pen(color, 1);
    pen.SetAlignment(PenAlignmentCenter);

    GraphicsPath path;

    createRoundRectanglePath(&path, r, dia);
    graphics->DrawPath(&pen, &path);

    for(int i=1; i<width; i++)
    {
		r.Inflate(-1, 0);
        createRoundRectanglePath(&path, r, dia);
            
        graphics->DrawPath(&pen, &path);

		r.Inflate(0, -1);

        createRoundRectanglePath(&path, r, dia);
        graphics->DrawPath(&pen, &path);
    }

    graphics->SetPageUnit((Unit)oldPageUnit);
}

void fillRoundRectangle(Gdiplus::Graphics* gr, Gdiplus::Brush* brush, const Gdiplus::Rect& r, const Gdiplus::Color& border, int radius)
{
    int dia = 2*radius;

    int oldPageUnit = gr->SetPageUnit(UnitPixel);

    Pen pen(border, 1);    
    pen.SetAlignment(PenAlignmentCenter);

    GraphicsPath path;
    createRoundRectanglePath(&path, r, dia);

    gr->FillPath(brush, &path);
    gr->DrawPath(&pen, &path);
    gr->SetPageUnit((Unit)oldPageUnit);
}


void drawEdit(Gdiplus::Graphics* gr, const Gdiplus::Rect& rect, const wchar_t* text, const Gdiplus::Font *font, bool pathEllipsis, bool grayBackground)
{
	using Gdiplus::Color;
    Gdiplus::Rect rt = rect;
    rt.Inflate( -1, -1 );
	
	Color lightColor = gdiplusSysColor(grayBackground ? COLOR_BTNFACE : COLOR_WINDOW);
	Color darkColor = gdiplusSysColor(grayBackground ? COLOR_3DSHADOW : COLOR_BTNFACE);
	LinearGradientBrush brush(Gdiplus::Rect(rt.X, rt.Y, rt.Width, rt.Height), Color(), Color(), LinearGradientModeVertical);

	Color colors[3] = { darkColor, lightColor, lightColor };
	Gdiplus::REAL positions[3] = { 0.0f, 0.4f, 1.0f };
	brush.SetInterpolationColors(colors, positions, 3);

	Color penColor;
	penColor.SetFromCOLORREF(GetSysColor(COLOR_3DSHADOW));
    fillRoundRectangle(gr, &brush, rt, penColor, 5);

	rt.Inflate(-3, -1);
	rt.Height -= 1;
	Rect textRect( rt.GetLeft(), rt.GetTop(), rt.GetRight(), rt.GetBottom() );
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	format.SetLineAlignment(StringAlignmentCenter);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(pathEllipsis ? StringTrimmingEllipsisPath : StringTrimmingEllipsisCharacter);
	
	SolidBrush textBrush(gdiplusSysColor(COLOR_WINDOWTEXT));
    gr->DrawString(text, (int)wcslen(text), font, gdiplusRectF(textRect), &format, &textBrush );
}

void drawCheck(Gdiplus::Graphics* gr, const Gdiplus::Rect& rect, bool checked)
{
	using Gdiplus::Color;
	int size = 17;

	int offsetY = ((rect.Height) - size) / 2;
	int offsetX = ((rect.Width) - size) / 2;

	Color brushColor;
	brushColor.SetFromCOLORREF(GetSysColor(COLOR_WINDOW));
	SolidBrush brush(brushColor);
	Color penColor;
	penColor.SetFromCOLORREF(GetSysColor(COLOR_3DSHADOW));
	fillRoundRectangle(gr, &brush, Gdiplus::Rect(rect.X + offsetX, rect.Y + offsetY, size, size), penColor, 4);

	if(checked){
		ASSERT(checkBitmap);
		HDC dc = gr->GetHDC();
		DrawState(dc, 0, 0, (LPARAM)checkBitmap, 0, rect.X + offsetX + 3, rect.Y + offsetY + 2, size - 5, size - 3, DST_BITMAP);
		gr->ReleaseHDC(dc);
	}
}

// ---------------------------------------------------------------------------

void PropertyDrawContext::drawCheck(const Rect& rect, bool grayed, bool checked) const
{
	if (grayed) {
		HDC dc = graphics->GetHDC();

		RECT rt = { rect.left(), rect.top(), rect.right(), rect.bottom() };
		Win32::drawGrayedCheck(dc, rt);
		graphics->ReleaseHDC(dc);
	}
	else {
		ww::drawCheck(graphics, gdiplusRect(rect), checked);
	}
}


void PropertyDrawContext::drawValueText(bool highlighted, const wchar_t* text) const
{
	Color textColor;
	textColor.setGDI(GetSysColor(highlighted ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));

	Rect textRect(widgetRect.left() + 3, widgetRect.top() + 2, widgetRect.right() - 3, widgetRect.bottom() - 2);

	tree->_drawRowValue(graphics, text, propertyTreeDefaultFont(), textRect, textColor, false, false);
}

void PropertyDrawContext::drawEntry(const wchar_t* text, bool pathEllipsis, bool grayBackground) const
{
	using Gdiplus::Color;
    Gdiplus::Rect rt = gdiplusRect(widgetRect);
	Gdiplus::Font* font = propertyTreeDefaultFont();
    rt.Inflate( -1, -1 );
	
	Color lightColor = gdiplusSysColor(grayBackground ? COLOR_BTNFACE : COLOR_WINDOW);
	Color darkColor = gdiplusSysColor(grayBackground ? COLOR_3DSHADOW : COLOR_BTNFACE);
	LinearGradientBrush brush(Gdiplus::Rect(rt.X, rt.Y, rt.Width, rt.Height), Color(), Color(), LinearGradientModeVertical);

	Color colors[3] = { darkColor, lightColor, lightColor };
	Gdiplus::REAL positions[3] = { 0.0f, 0.4f, 1.0f };
	brush.SetInterpolationColors(colors, positions, 3);

	Color penColor;
	penColor.SetFromCOLORREF(GetSysColor(COLOR_3DSHADOW));
    fillRoundRectangle(graphics, &brush, rt, penColor, 5);

	rt.Inflate(-3, -1);
	rt.Height -= 1;
	Rect textRect( rt.GetLeft(), rt.GetTop(), rt.GetRight(), rt.GetBottom() );
	
	ww::Color color;
	color.setGDI(GetSysColor(COLOR_WINDOWTEXT));
	tree->_drawRowValue(graphics, text, font, textRect, color, pathEllipsis, false);
    //gr->DrawString(text, (int)wcslen(text), font, gdiplusRectF(textRect), gdiplusSysColor(COLOR_WINDOWTEXT) );
}

}
