/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyRowImpl.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "ww/Decorators.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTreeModel.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "gdiplusUtils.h"

namespace ww{

class PropertyRowButton : public PropertyRowImpl<ButtonDecorator, PropertyRowButton>{
public:
	PropertyRowButton();
	void redraw(const PropertyDrawContext& context);
	bool onMouseDown(PropertyTree* tree, Vect2 point, bool& changed);
	void onMouseMove(PropertyTree* tree, Vect2 point);
	void onMouseUp(PropertyTree* tree, Vect2 point);
	bool onActivate(PropertyTree* tree, bool force);
	int floorHeight() const{ return 3; }
	string valueAsString() const { return value_ ? value_.text : ""; }
	int widgetSizeMin() const{ 
		if (userWidgetSize() >= 0)
			return userWidgetSize();
		else
			return 60; 
	}
protected:
	bool underMouse_;
	bool locked_;
};

PropertyRowButton::PropertyRowButton()
: underMouse_(false), locked_(false)
{
}
	

void PropertyRowButton::redraw(const PropertyDrawContext& context)
{
	using namespace Gdiplus;
	using Gdiplus::Color;

	Rect buttonRect(context.widgetRect);
	buttonRect.setLeft(buttonRect.left() - 1);
	buttonRect.setRight(buttonRect.right() + 1);
	buttonRect.setBottom(buttonRect.bottom() + 2);

	wstring text(toWideChar(value().text ? value().text : labelUndecorated()));
	bool pressed = underMouse_ && value();
	context.drawButton(buttonRect, text.c_str(), pressed, selected() && context.tree->hasFocus());
}

bool PropertyRowButton::onMouseDown(PropertyTree* tree, Vect2 point, bool& changed)
{
	if(widgetRect().pointInside(point)){
		value().pressed = !value().pressed;
		underMouse_ = true;
		tree->redraw();
		return true;
	}
	return false;
}

void PropertyRowButton::onMouseMove(PropertyTree* tree, Vect2 point)
{
	bool underMouse = widgetRect().pointInside(point);
	if(underMouse != underMouse_){
		underMouse_ = underMouse;
		tree->redraw();
	}
}

void PropertyRowButton::onMouseUp(PropertyTree* tree, Vect2 point)
{
	if(!locked_ && widgetRect().pointInside(point)){
		onActivate(tree, false);
    }
	else{
        tree->model()->rowAboutToBeChanged(this);
		value().pressed = false;
		tree->redraw();
	}
}

bool PropertyRowButton::onActivate(PropertyTree* tree, bool force)
{
	value().pressed = true;
	locked_ = true;
	tree->model()->rowChanged(this); // Row is recreated here, so don't unlock
	return true;
}

DECLARE_SEGMENT(PropertyRowDecorators)
REGISTER_PROPERTY_ROW(ButtonDecorator, PropertyRowButton);

// ------------------------------------------------------------------------------------------

class PropertyRowHLine : public PropertyRowImpl<HLineDecorator, PropertyRowHLine>{
public:
	void redraw(const PropertyDrawContext& context);
	bool isSelectable() const{ return false; }
};

void PropertyRowHLine::redraw(const PropertyDrawContext& context)
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
	void redraw(const PropertyDrawContext& context);
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

void PropertyRowNot::redraw(const PropertyDrawContext& context)
{
	Win32::drawNotCheck(context.graphics, gdiplusRect(context.widgetRect), value());
}

REGISTER_PROPERTY_ROW(NotDecorator, PropertyRowNot);

// ---------------------------------------------------------------------------
//
class PropertyRowRadio : public PropertyRowImpl<RadioDecorator, PropertyRowRadio>{
public:
	bool onActivate(PropertyTree* tree, bool force);
	void redraw(const PropertyDrawContext& context);
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

void PropertyRowRadio::redraw(const PropertyDrawContext& context)
{
	Win32::drawRadio(context.graphics, gdiplusRect(context.widgetRect), value());
}

REGISTER_PROPERTY_ROW(RadioDecorator, PropertyRowRadio);

}
