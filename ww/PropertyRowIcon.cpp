/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/TypesFactory.h"

#include "ww/PropertyDrawContext.h"
#include "ww/PropertyRowImpl.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyTreeModel.h"
#include "ww/Serialization.h"

#include "ww/Win32/Drawing.h"
#include "ww/Color.h"	
#include "ww/Icon.h"
#include "gdiplus.h"
using std::wstring;

namespace ww{


class PropertyRowIcon : public PropertyRow{
public:
	static const bool Custom = true;

	PropertyRowIcon()
	{
	}

	PropertyRowIcon(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
	: PropertyRow(name, nameAlt, typeName)
	{
		icon_ = *(Icon*)(object);
		WW_VERIFY(icon_.getImage(&image_));
	}

	bool assignTo(void* val, size_t size)
	{
		return false;
	}

	void redraw(const PropertyDrawContext& context)
	{
		Rect rect = context.widgetRect;
		Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(image_.width_, image_.height_, image_.width_ * 4, PixelFormat32bppARGB, (BYTE*)&image_.pixels_[0]);
		int x = rect.left();
		int y = rect.top() + (ROW_DEFAULT_HEIGHT - image_.height_) / 2;
		context.graphics->DrawImage(bitmap, x, y);
		delete bitmap;
	}

	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return false; }

	bool onActivate(PropertyTree* tree, bool force)
	{

		return false;
	}
	void digestReset() {}
	wstring valueAsWString() const{ return L""; }
	wstring digestValue() const { return wstring(); }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	PropertyRow* clone() const{
		return cloneChildren(new PropertyRowIcon((void*)&icon_, sizeof(icon_), name_, label_, typeid(Icon).name()), this);
	}
	void serializeValue(Archive& ar) {}
	int widgetSizeMin() const{ return image_.width_; }
	int height() const{ return image_.height_; }
protected:
	RGBAImage image_;
	Icon icon_;
};

class PropertyRowIconToggle : public PropertyRowImpl<IconToggle, PropertyRowIconToggle>{
public:
	static const bool Custom = true;

	PropertyRowIconToggle()
	{
	}

	PropertyRowIconToggle(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
	: PropertyRowImpl<IconToggle, PropertyRowIconToggle>(object, size, name, nameAlt, typeName)
	{
		WW_VERIFY(value().iconTrue_.getImage(&imageTrue_));
		WW_VERIFY(value().iconFalse_.getImage(&imageFalse_));
	}

	void redraw(const PropertyDrawContext& context)
	{
		Rect rect = context.widgetRect;
		RGBAImage& image = value().value_ ? imageTrue_ : imageFalse_;
		Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(image.width_, image.height_, image.width_ * 4, PixelFormat32bppARGB, (BYTE*)&image.pixels_[0]);
		int x = rect.left();
		int y = rect.top() + (ROW_DEFAULT_HEIGHT - image.height_) / 2;
		context.graphics->DrawImage(bitmap, x, y);
		delete bitmap;
	}

	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return true; }

	bool onActivate(PropertyTree* tree, bool force)
	{
		tree->model()->push(this);
		value().value_ = !value().value_;
		tree->model()->rowChanged(this);
		return true;
	}
	void digestReset() {}
	wstring valueAsWString() const{ return L""; }
	wstring digestValue() const { return wstring(); }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }

	int widgetSizeMin() const{ return imageFalse_.width_ + 1; }
	int height() const{ return imageFalse_.height_; }
protected:
	RGBAImage imageTrue_;
	RGBAImage imageFalse_;
	IconToggle iconToggle_;
};

REGISTER_PROPERTY_ROW(Icon, PropertyRowIcon); 
REGISTER_PROPERTY_ROW(IconToggle, PropertyRowIconToggle); 
DECLARE_SEGMENT(PropertyRowIcon)
}
