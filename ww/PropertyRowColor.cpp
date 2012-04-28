/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/ClassFactory.h"
#include "ww/PropertyTreeModel.h"
#include "ww/_PropertyRowBuiltin.h"

#include "ww/PropertyTree.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyRow.h"
#include "ww/ColorChooserDialog.h"
#include "ww/Serialization.h"

#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window.h"
#include "ww/Color.h"	
#ifndef WW_DISABLE_XMATH
#include "XMath/Colors.h"
#endif
//#include "XMath/Streams.h"
#include "gdiplus.h"

namespace ww{


template<size_t Size>
void formatColor(char (&str)[Size], const ww::Color& c) { sprintf_s(str, "(%i %i %i %i)", c.r, c.g, c.b, c.a); }

#ifndef WW_DISABLE_XMATH
template<size_t Size>
void formatColor(char (&str)[Size], const Color4c& c) { sprintf_s(str, sizeof(str), "(%i %i %i %i)", c.r, c.g, c.b, c.a); }
template<size_t Size>
void formatColor(char (&str)[Size], const Color3c& c) { sprintf_s(str, sizeof(str), "(%i %i %i)", c.r, c.g, c.b); }
template<size_t Size>
void formatColor(char (&str)[Size], const Color4f& c) { sprintf_s(str, sizeof(str), "(%g %g %g %g)", c.r, c.g, c.b, c.a); }

#endif



unsigned int toARGB(Color color)
{
	return color.argb();
}

#ifndef WW_DISABLE_XMATH
unsigned int toARGB(Color4c color)
{
	return color.argb();
}

unsigned int toARGB(const Color4f& color)
{
	return Color4c(color).argb();
}

#endif

void fromColor(Color* out, Color color)
{
	*out = color;
}

#ifndef WW_DISABLE_XMATH
void fromColor(Color4c* out, Color color)
{
	out->set(color.r, color.g, color.b, color.a);
}

void fromColor(Color4f* out, Color color)
{
	*out = Color4f(Color4c(color.r, color.g, color.b, color.a));
}


void fromColor(Color3c* out, Color color)
{
	out->set(color.r, color.g, color.b);
}
#endif

template<class ColorType>
class PropertyRowColor : public PropertyRowImpl<ColorType, PropertyRowColor<ColorType> >{
public:
	static const bool Custom = true;
	PropertyRowColor(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	PropertyRowColor();
	void redraw(const PropertyDrawContext& context);

	bool activateOnAdd() const{ return true; }
	bool onActivate(PropertyTree* tree, bool force);
	std::string valueAsString() const { 
		char buf[64];
		formatColor(buf, value_);
		return string(buf);
	}
	virtual int widgetSizeMin() const{ 
		return userWidgetSize() ? userWidgetSize() : ICON_SIZE; 
	}
};

template<class ColorType>
bool PropertyRowColor<ColorType>::onActivate(PropertyTree* tree, bool force)
{
	ColorChooserDialog dialog(tree, Color(toARGB(value())));
    if(dialog.showModal() == ww::RESPONSE_OK){
        tree->model()->push(this);
		fromColor(&value(), dialog.get());
		tree->model()->rowChanged(this);
		return true;
	}
	return false;
}


template<class ColorType>
PropertyRowColor<ColorType>::PropertyRowColor(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<ColorType, PropertyRowColor>(object, size, name, nameAlt, typeName)
{
}

template<class ColorType>
void PropertyRowColor<ColorType>::redraw(const PropertyDrawContext& context)
{
	using namespace Gdiplus;
	using Gdiplus::Color;

	if(multiValue()){
		__super::redraw(context);
		return;
	}

	Rect rect = context.widgetRect;
	rect.setLeft(rect.left() + 1);
	rect.setHeight(rect.height() - 1);
	Gdiplus::Color color(toARGB(value()));
	SolidBrush brush(color);
	Color penColor;
	penColor.SetFromCOLORREF(GetSysColor(COLOR_3DSHADOW));
	
	HatchBrush hatchBrush(HatchStyleSmallCheckerBoard, Color::Black, Color::White);

	if (rect.width() > ICON_SIZE * 2 + 5){
		Rect rectb(rect.right() - ICON_SIZE - 3, rect.top(), rect.right(), rect.bottom());
		fillRoundRectangle(context.graphics, &hatchBrush, gdiplusRect(rectb), Color(0, 0, 0, 0), 6);
		fillRoundRectangle(context.graphics, &brush, gdiplusRect(rectb), penColor, 6);

		SolidBrush brushb(Color(255, color.GetR(), color.GetG(), color.GetB()));
		Rect recta(rect.left(), rect.top(), rect.right() - ICON_SIZE - 5, rect.bottom());
		fillRoundRectangle(context.graphics, &brushb, gdiplusRect(recta), penColor, 6);
	
	}
	else{
		fillRoundRectangle(context.graphics, &hatchBrush, gdiplusRect(rect), Color(0, 0, 0, 0), 6);
		fillRoundRectangle(context.graphics, &brush, gdiplusRect(rect), penColor, 6);
	}
}

template<class ColorType>
PropertyRowColor<ColorType>::PropertyRowColor()
{
}

DECLARE_SEGMENT(PropertyRowColor)
typedef PropertyRowColor<Color> PropertyRowWWColor;
REGISTER_PROPERTY_ROW(Color, PropertyRowWWColor); 
#ifndef WW_DISABLE_XMATH
typedef PropertyRowColor<Color3c> PropertyRowColor3c;
REGISTER_PROPERTY_ROW(Color3c, PropertyRowColor3c); 
typedef PropertyRowColor<Color4c> PropertyRowColor4c;
REGISTER_PROPERTY_ROW(Color4c, PropertyRowColor4c); 
typedef PropertyRowColor<Color4f> PropertyRowColor4f;
REGISTER_PROPERTY_ROW(Color4f, PropertyRowColor4f); 
#endif
}
