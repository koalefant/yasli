/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/ClassFactory.h"

#include "PropertyDrawContext.h"
#include "PropertyRowImpl.h"
#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "Serialization.h"
#include "Color.h"	
#include "yasli/decorators/IconXPM.h"
using yasli::IconXPM;
using yasli::IconXPMToggle;

class PropertyRowIconXPM : public PropertyRow{
public:
	static const bool Custom = true;

	bool assignTo(void* val, size_t size)
	{
		return false;
	}

	void redraw(const PropertyDrawContext& context)
	{
		QRect rect = context.widgetRect;
		context.drawIcon(rect, icon_);
	}

	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return false; }

	bool onActivate(QPropertyTree* tree, bool force)
	{

		return false;
	}
	void setValue(const Serializer& ser) override {
		YASLI_ESCAPE(ser.size() == sizeof(IconXPM), return);
		icon_ = *(IconXPM*)(ser.pointer());
	}
	yasli::wstring valueAsWString() const{ return L""; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	PropertyRow* clone() const{
		PropertyRowIconXPM* result = new PropertyRowIconXPM();
		result->setNames(name_, label_, typeName_);
		result->icon_ = icon_;
		return cloneChildren(result, this);
	}
	void serializeValue(Archive& ar) {}
	int widgetSizeMin() const{ return 18; }
	int height() const{ return 16; }
protected:
	IconXPM icon_;
};

class PropertyRowIconToggle : public PropertyRowImpl<IconXPMToggle, PropertyRowIconToggle>{
public:
	static const bool Custom = true;

	void redraw(const PropertyDrawContext& context)
	{
		IconXPM& icon = value().value_ ? value().iconTrue_ : value().iconFalse_;
		context.drawIcon(context.widgetRect, icon);
	}

	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return true; }
	bool onActivate(QPropertyTree* tree, bool force)
	{
		tree->model()->rowAboutToBeChanged(this);
		value().value_ = !value().value_;
		tree->model()->rowChanged(this);
		return true;
	}
	yasli::wstring valueAsWString() const{ return value().value_ ? L"true" : L"false"; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }

	int widgetSizeMin() const{ return 18; }
	int height() const{ return 16; }
};

REGISTER_PROPERTY_ROW(IconXPM, PropertyRowIconXPM); 
REGISTER_PROPERTY_ROW(IconXPMToggle, PropertyRowIconToggle); 
DECLARE_SEGMENT(PropertyRowIconXPM)
