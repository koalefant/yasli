#include "StdAfx.h"
#include "_PropertyRowBuiltin.h"
#include "PropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyTreeDrawing.h"
#include "Unicode.h"
#include "Serialization.h"
#include "Win32/Rectangle.h"
#include "Win32/Window.h"
#include "gdiplus.h"

namespace ww{

YASLI_CLASS(PropertyRow, PropertyRowBool, "bool");

PropertyRowBool::PropertyRowBool(const char* name, const char* label, bool value)
: PropertyRow(name, label, "bool")
, value_(value)
{
}

bool PropertyRowBool::assignTo(void* object, size_t size)
{
	ASSERT(size == sizeof(bool));
	*reinterpret_cast<bool*>(object) = value_;
	return true;
}

void PropertyRowBool::redraw(Gdiplus::Graphics* gr, const Gdiplus::Rect& _widgetRect, const Gdiplus::Rect& floorRect)
{
	using namespace Gdiplus;
	if(multiValue())
	{
		HDC dc = gr->GetHDC();
		Win32::drawGrayedCheck(dc, widgetRect().rect());
		gr->ReleaseHDC(dc);
	}
	else
		drawCheck(gr, gdiplusRect(widgetRect()), value_);
}

bool PropertyRowBool::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->push(this);
	value_ = !value_;
	tree->model()->rowChanged(this);
	return true;
}

void PropertyRowBool::serializeValue(Archive& ar)
{
    ar(value_, "value", "Value");
}

// ---------------------------------------------------------------------------

int PropertyRowPointer::widgetSizeMin() const
{
    HDC dc = GetDC(*Win32::_globalDummyWindow);
    Gdiplus::Graphics gr(dc);
    Gdiplus::Font* font = propertyTreeDefaultBoldFont();
    std::wstring wstr(generateLabel());
    Gdiplus::StringFormat format;
    Gdiplus::RectF bound;
    gr.MeasureString(wstr.c_str(), (int)wstr.size(), font, Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f), &format, &bound, 0);
    ReleaseDC(*Win32::_globalDummyWindow, dc);
    return bound.Width + 10;
}


}
