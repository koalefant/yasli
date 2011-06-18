#include "StdAfx.h"
#include "yasli/TypesFactory.h"
#include "ww/PropertyTreeModel.h"
#include "ww/_PropertyRowBuiltin.h"

#include "ww/PropertyTree.h"
#include "ww/PropertyTreeDrawing.h"
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
	PropertyRowColor(void* object, int size, const char* name, const char* nameAlt, const char* typeName);
	PropertyRowColor();
	void redraw(Gdiplus::Graphics *gr, const Gdiplus::Rect& widgetRect, const Gdiplus::Rect& lineRect);

	bool activateOnAdd() const{ return true; }
	bool onActivate(PropertyTree* tree, bool force);
	std::string valueAsString() const { return (MemoryWriter() << toARGB(value_)).c_str(); }
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
PropertyRowColor<ColorType>::PropertyRowColor(void* object, int size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<ColorType, PropertyRowColor>(object, size, name, nameAlt, typeName)
{
	widgetSizeMin_ = 26;
}

template<class ColorType>
void PropertyRowColor<ColorType>::redraw(Gdiplus::Graphics* gr, const Gdiplus::Rect& widgetRect, const Gdiplus::Rect& floorRect)
{
	using namespace Gdiplus;
	using Gdiplus::Rect;
	using Gdiplus::Color;

	if(multiValue()){
		__super::redraw(gr, widgetRect, floorRect);
		return;
	}

	Rect rect = widgetRect;
	rect.Y += 1;
	rect.Height -= 2;
	Gdiplus::Color color(toARGB(value()));
	SolidBrush brush(color);
	Color penColor;
	penColor.SetFromCOLORREF(GetSysColor(COLOR_3DSHADOW));
	
	HatchBrush hatchBrush(HatchStyleSmallCheckerBoard, Color::Black, Color::White);

	Rect rectb(rect.GetRight() - ICON_SIZE - 3, rect.Y, ICON_SIZE, rect.Height);
	fillRoundRectangle(gr, &hatchBrush, rectb, Color(0, 0, 0, 0), 6);
	fillRoundRectangle(gr, &brush, rectb, penColor, 6);

	SolidBrush brushb(Color(255, color.GetR(), color.GetG(), color.GetB()));
	Rect recta(rect.X, rect.Y, rect.Width - ICON_SIZE - 5, rect.Height);
	fillRoundRectangle(gr, &brushb, recta, penColor, 6);
}

template<class ColorType>
PropertyRowColor<ColorType>::PropertyRowColor()
{
	widgetSizeMin_ = 26;
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
