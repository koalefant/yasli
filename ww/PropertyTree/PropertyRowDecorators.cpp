/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "ww/Decorators.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/IDrawContext.h"
#include "PropetryTree/PropertyTreeModel.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "gdiplusUtils.h"
#include "yasli/decorators/Button.h"

namespace property_tree{

DECLARE_SEGMENT(PropertyRowDecorators)

// ------------------------------------------------------------------------------------------

class PropertyRowHLine : public PropertyRowImpl<HLineDecorator, PropertyRowHLine>{
public:
	void redraw(const PropertyDrawContext& context);
	bool isSelectable() const{ return false; }
};

void PropertyRowHLine::redraw(const IDrawContext& context)
{
	int halfHeight = context.widgetRect.top() + (context.widgetRect.height()) / 2;
	RECT hlineRect = { context.widgetRect.left(), halfHeight - 1, context.widgetRect.right(), halfHeight + 1 };

	HDC dc = context.graphics->GetHDC();
	DrawEdge(dc, &hlineRect, BDR_SUNKENOUTER, BF_RECT);
	context.graphics->ReleaseHDC(dc);
}

REGISTER_PROPERTY_ROW(HLineDecorator, PropertyRowHLine);

// ------------------------------------------------------------------------------------------

class PropertyRowNot : public PropertyRowImpl<NotDecorator, PropertyRowNot>{
public:
	bool onActivate(PropertyTree* tree, bool force);
	void redraw(const IDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	string valueAsString() const { return value_ ? label() : ""; }
	virtual int widgetSizeMin() const{ return ICON_SIZE; }
};

bool PropertyRowNot::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->rowAboutToBeChanged(this);
	value() = !value();
	tree->model()->rowChanged(this);
	return true;
}

void PropertyRowNot::redraw(const IDrawContext& context)
{
	Win32::drawNotCheck(context.graphics, gdiplusRect(context.widgetRect), value());
}

REGISTER_PROPERTY_ROW(NotDecorator, PropertyRowNot);

// ---------------------------------------------------------------------------
//
class PropertyRowRadio : public PropertyRowImpl<RadioDecorator, PropertyRowRadio>{
public:
	bool onActivate(PropertyTree* tree, bool force);
	void redraw(const IDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	string valueAsString() const { return value() ? label() : ""; }
	int widgetSizeMin() const{ return ICON_SIZE; }
};

bool PropertyRowRadio::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->rowAboutToBeChanged(this);
	value() = !value();
	tree->model()->rowChanged(this);
	return true;
}

void PropertyRowRadio::redraw(const IDrawContext& context)
{
	Win32::drawRadio(context.graphics, gdiplusRect(context.widgetRect), value());
}

REGISTER_PROPERTY_ROW(RadioDecorator, PropertyRowRadio);

}
