/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "_PropertyRowBuiltin.h"
#include "PropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "Unicode.h"
#include "Serialization.h"
#include "Win32/Rectangle.h"
#include "Win32/Window32.h"
#include "gdiplusUtils.h"

namespace ww{

YASLI_CLASS(PropertyRow, PropertyRowBool, "bool");

PropertyRowBool::PropertyRowBool(const char* name, const char* label, bool value)
: PropertyRow(name, label, "bool")
, value_(value)
{
}

bool PropertyRowBool::assignTo(void* object, size_t size)
{
	YASLI_ASSERT(size == sizeof(bool));
	*reinterpret_cast<bool*>(object) = value_;
	return true;
}

void PropertyRowBool::redraw(const PropertyDrawContext& context)
{
	context.drawCheck(widgetRect(), userReadOnly(), multiValue() ? CHECK_IN_BETWEEN : (value_ ? CHECK_SET : CHECK_NOT_SET));
}

bool PropertyRowBool::onActivate(PropertyTree* tree, bool force)
{
	if (!this->userReadOnly()) {
		tree->model()->push(this);
		value_ = !value_;
		tree->model()->rowChanged(this);
		return true;
	}
	else
	return false;
}

void PropertyRowBool::digestReset(const PropertyTree* tree)
{
	digest_ = value_ ? toWideChar(labelUndecorated()) : L"";
}

void PropertyRowBool::serializeValue(Archive& ar)
{
    ar(value_, "value", "Value");
}

// ---------------------------------------------------------------------------

int PropertyRowPointer::widgetSizeMin() const
{
    HDC dc = GetDC(Win32::getDefaultWindowHandle());
    Gdiplus::Graphics gr(dc);
    Gdiplus::Font* font = propertyTreeDefaultBoldFont();
    wstring wstr(generateLabel());
    Gdiplus::StringFormat format;
    Gdiplus::RectF bound;
    gr.MeasureString(wstr.c_str(), (int)wstr.size(), font, Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f), &format, &bound, 0);
    ReleaseDC(Win32::getDefaultWindowHandle(), dc);
    return bound.Width + 10;
}


}
