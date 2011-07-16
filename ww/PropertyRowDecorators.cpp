#include "StdAfx.h"
#include "PropertyRowImpl.h"
#include "yasli/Archive.h"
#include "yasli/TypesFactory.h"
#include "ww/Decorators.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTreeModel.h"
#include "ww/Win32/Window.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "gdiplus.h"

namespace ww{

class PropertyRowButton : public PropertyRowImpl<ButtonDecorator, PropertyRowButton>{
public:
	PropertyRowButton();
	PropertyRowButton(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	void redraw(const PropertyDrawContext& context);
	bool onMouseDown(PropertyTree* tree, Vect2 point, bool& changed);
	void onMouseMove(PropertyTree* tree, Vect2 point);
	void onMouseUp(PropertyTree* tree, Vect2 point);
	bool onActivate(PropertyTree* tree, bool force);
	int floorHeight() const{ return 3; }
	std::string valueAsString() const { return value_ ? value_.text : ""; }
	int widgetSizeMin() const{ 
		if (userWidgetSize())
			return userWidgetSize();
		else
			return 60; 
	}
protected:
	bool underMouse_;
	bool locked_;
};

PropertyRowButton::PropertyRowButton(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<ButtonDecorator, PropertyRowButton>(object, size, name, nameAlt, typeName)
, underMouse_(false), locked_(false)
{
}

PropertyRowButton::PropertyRowButton()
: underMouse_(false), locked_(false)
{
}
	

void PropertyRowButton::redraw(const PropertyDrawContext& context)
{
	using namespace Gdiplus;
	using Gdiplus::Rect;
	using Gdiplus::Color;
	bool pressed = underMouse_ && value();
	Rect buttonRect = gdiplusRect(context.widgetRect);
	Graphics* gr = context.graphics;
	buttonRect.X -= 1;
	buttonRect.Width += 1;
	buttonRect.Height += 2;

	Color brushColor;
	brushColor.SetFromCOLORREF(GetSysColor(COLOR_BTNFACE));
	Gdiplus::SolidBrush brush(brushColor);
	gr->FillRectangle(&brush, buttonRect);

	RECT edgeRect = { buttonRect.X, buttonRect.Y, buttonRect.GetRight(), buttonRect.GetBottom() };
	HDC dc =  gr->GetHDC();
	DrawEdge(dc, &edgeRect, pressed ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT);
	gr->ReleaseHDC(dc);
	std::wstring text(toWideChar(value().text ? value().text : labelUndecorated()));

	Rect textRect = gdiplusRect(context.widgetRect);
	textRect.X += 2;
	textRect.Y += 1;
	textRect.Width -= 4;
	if(pressed){
		textRect.X += 1;
		textRect.Y += 1;
	}
	
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(StringTrimmingEllipsisCharacter);
	Color textColor;
	textColor.SetFromCOLORREF(GetSysColor(COLOR_WINDOWTEXT));
	SolidBrush textBrush(textColor);
	gr->DrawString( text.c_str(), (int)wcslen(text.c_str()), propertyTreeDefaultFont(), RectF(Gdiplus::REAL(textRect.X), Gdiplus::REAL(textRect.Y), Gdiplus::REAL(textRect.Width), Gdiplus::REAL(textRect.Height)), &format, &textBrush );
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
        tree->model()->push(this);
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
	PropertyRowHLine();
	PropertyRowHLine(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	void redraw(const PropertyDrawContext& context);
	bool isSelectable() const{ return false; }
};

PropertyRowHLine::PropertyRowHLine(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<HLineDecorator, PropertyRowHLine>(object, size, name, nameAlt, typeName)
{
}

PropertyRowHLine::PropertyRowHLine()
{
}

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
	PropertyRowNot();
	PropertyRowNot(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	bool onActivate(PropertyTree* tree, bool force);
	void redraw(const PropertyDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	std::string valueAsString() const { return value_ ? label() : ""; }
	virtual int widgetSizeMin() const{ return ICON_SIZE; }
};

PropertyRowNot::PropertyRowNot(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<NotDecorator, PropertyRowNot>(object, size, name, nameAlt, typeName)
{
}

PropertyRowNot::PropertyRowNot()
{
}
	
bool PropertyRowNot::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->push(this);
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
	PropertyRowRadio();
	PropertyRowRadio(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	bool onActivate(PropertyTree* tree, bool force);
	void redraw(const PropertyDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	std::string valueAsString() const { return value() ? label() : ""; }
	int widgetSizeMin() const{ return ICON_SIZE; }
};

PropertyRowRadio::PropertyRowRadio(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<RadioDecorator, PropertyRowRadio>(object, size, name, nameAlt, typeName)
{
}

PropertyRowRadio::PropertyRowRadio()
{
}
	
bool PropertyRowRadio::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->push(this);
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
