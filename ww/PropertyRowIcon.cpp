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

#include "ww/PropertyDrawContext.h"
#include "ww/PropertyRowImpl.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyTreeModel.h"
#include "ww/Serialization.h"

#include "ww/Win32/Drawing.h"
#include "ww/Color.h"	
#include "ww/Icon.h"
#include "gdiplusUtils.h"

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
		YASLI_ESCAPE(size == sizeof(Icon), return);
		icon_ = *(Icon*)(object);
	}

	bool assignTo(void* val, size_t size)
	{
		return false;
	}

	void redraw(const PropertyDrawContext& context)
	{
		Rect rect = context.widgetRect;
		context.drawIcon(rect, icon_);
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
	int widgetSizeMin() const{ return icon_.width() + 2; }
	int height() const{ return icon_.height(); }
protected:
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
	}

	void redraw(const PropertyDrawContext& context)
	{
		Icon& icon = value().value_ ? value().iconTrue_ : value().iconFalse_;
		context.drawIcon(context.widgetRect, icon);
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
	void digestReset(const PropertyTree* tree) {}
	wstring valueAsWString() const{ return value().value_ ? L"true" : L"false"; }
	wstring digestValue() const { return wstring(); }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }

	int widgetSizeMin() const{ return value().iconFalse_.width() + 1; }
	int height() const{ return value().iconFalse_.height(); }
};

REGISTER_PROPERTY_ROW(Icon, PropertyRowIcon); 
REGISTER_PROPERTY_ROW(IconToggle, PropertyRowIconToggle); 
DECLARE_SEGMENT(PropertyRowIcon)
}
