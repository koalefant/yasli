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

	bool onActivate(PropertyTree* tree, bool rename)
	{

		return false;
	}
	void setValue(const Serializer& ser) override {
		YASLI_ESCAPE(ser.size() == sizeof(Icon), return);
		icon_ = *(Icon*)(ser.pointer());
	}
	string valueAsString() const{ return ""; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	PropertyRow* clone() const{
		PropertyRowIcon* result = new PropertyRowIcon();
		result->setNames(name_, label_, typeName_);
		result->icon_ = icon_;
		return cloneChildren(result, this);
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
		tree->model()->rowAboutToBeChanged(this);
		value().value_ = !value().value_;
		tree->model()-rowChanged(this);
		return true;
	}
	string valueAsString() const{ return value().value_ ? "true" : "false"; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }

	int widgetSizeMin() const{ return value().iconFalse_.width() + 1; }
	int height() const{ return value().iconFalse_.height(); }
};

REGISTER_PROPERTY_ROW(Icon, PropertyRowIcon); 
REGISTER_PROPERTY_ROW(IconToggle, PropertyRowIconToggle); 
DECLARE_SEGMENT(PropertyRowIcon)
}
