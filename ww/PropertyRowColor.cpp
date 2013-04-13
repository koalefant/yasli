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
#include "ww/PropertyRowImpl.h"

#include "ww/PropertyTree.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyRow.h"
#include "ww/ColorChooserDialog.h"
#include "ww/Serialization.h"

#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window32.h"
#include "ww/Color.h"	
#include "gdiplusUtils.h"

namespace ww{


template<size_t Size>
void formatColor(char (&str)[Size], const ww::Color& c) { sprintf_s(str, "(%i %i %i %i)", c.r, c.g, c.b, c.a); }

unsigned int toARGB(Color color)
{
	return color.argb();
}

void fromColor(Color* out, Color color)
{
	*out = color;
}

template<class ColorType>
class PropertyRowColor : public PropertyRowImpl<ColorType, PropertyRowColor<ColorType> >{
public:
	static const bool Custom = true;
	PropertyRowColor();
	void redraw(const PropertyDrawContext& context);

	bool activateOnAdd() const{ return true; }
	bool onActivate(PropertyTree* tree, bool force);
	string valueAsString() const { 
		char buf[64];
		formatColor(buf, value_);
		return string(buf);
	}
	virtual int widgetSizeMin() const{ 
		return userWidgetSize() >= 0 ? userWidgetSize() : ICON_SIZE; 
	}
};

template<class ColorType>
bool PropertyRowColor<ColorType>::onActivate(PropertyTree* tree, bool force)
{
	ColorChooserDialog dialog(tree, Color(toARGB(value())));
    if(dialog.showModal() == ww::RESPONSE_OK){
        tree->model()->rowAboutToBeChanged(this);
		fromColor(&value(), dialog.get());
		tree->model()->rowChanged(this);
		return true;
	}
	return false;
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
}
