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
	void redraw(PropertyDrawContext& context) override
	{
		context.drawIcon(context.widgetRect, icon_);
	}

	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return false; }

	bool onActivate(PropertyTree* tree, bool force) override
	{
		return false;
	}
	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		YASLI_ESCAPE(ser.size() == sizeof(IconXPM), return);
		icon_ = *(IconXPM*)(ser.pointer());
	}
	yasli::wstring valueAsWString() const{ return L""; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	void serializeValue(Archive& ar) {}
	int widgetSizeMin(const PropertyTree*) const override{ return 18; }
	int height() const{ return 16; }
protected:
	IconXPM icon_;
};

class PropertyRowIconToggle : public PropertyRow{
public:
	void redraw(PropertyDrawContext& context) override
	{
		IconXPM& icon = value_ ? iconTrue_ : iconFalse_;
		context.drawIcon(context.widgetRect, icon);
	}

	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		YASLI_ESCAPE(ser.size() == sizeof(IconXPMToggle), return);
		const IconXPMToggle* icon = (IconXPMToggle*)(ser.pointer());
		iconTrue_ = icon->iconTrue_;
		iconFalse_ = icon->iconFalse_;
		value_ = icon->value_;
	}

	bool assignTo(const yasli::Serializer& ser) const override
	{
		IconXPMToggle* toggle = (IconXPMToggle*)ser.pointer();
		toggle->value_ = value_;
		return true;
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	bool isSelectable() const override{ return true; }
	bool onActivate(PropertyTree* tree, bool force) override
	{
		tree->model()->rowAboutToBeChanged(this);
		value_ = !value_;
		tree->model()->rowChanged(this);
		return true;
	}
	DragCheckBegin onMouseDragCheckBegin() override
	{
		if (userReadOnly())
			return DRAG_CHECK_IGNORE;
		return value_ ? DRAG_CHECK_UNSET : DRAG_CHECK_SET;
	}
	bool onMouseDragCheck(PropertyTree* tree, bool value) override
	{
		if (value_ != value) {
			tree->model()->rowAboutToBeChanged(this);
			value_ = value;
			tree->model()->rowChanged(this);
			return true;
		}
		return false;
	}
	yasli::wstring valueAsWString() const{ return value_ ? L"true" : L"false"; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }

	int widgetSizeMin(const PropertyTree*) const override{ return 18; }
	int height() const{ return 16; }

	IconXPM iconTrue_;
	IconXPM iconFalse_;
	bool value_;
};

REGISTER_PROPERTY_ROW(IconXPM, PropertyRowIconXPM); 
REGISTER_PROPERTY_ROW(IconXPMToggle, PropertyRowIconToggle); 
DECLARE_SEGMENT(PropertyRowIconXPM)
