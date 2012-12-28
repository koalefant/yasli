/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <math.h>

#include "ww/PropertyRowStringListValue.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTree.h"

#include "ww/ConstStringList.h"
#include "ww/PopupMenu.h"
#include "ww/ComboBox.h"
#include "ww/Entry.h"
#include "ClassMenu.h"

#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "gdiplusUtils.h"

namespace ww{

class AutoDropComboBox : public ComboBox{
public:
	void show(){
		ComboBox::show();
		ComboBox::setFocus();
		showDropDown();
	}
};

// ---------------------------------------------------------------------------
REGISTER_PROPERTY_ROW(StringListValue, PropertyRowStringListValue)

class PropertyRowWidgetStringListValue : public PropertyRowWidget, public has_slots{
public:
	PropertyRowWidgetStringListValue(PropertyRowStringListValue* row, PropertyTreeModel* model)
	: PropertyRowWidget(row, model)
	, comboBox_(new AutoDropComboBox())
	{
		comboBox_->set(row->value(), false);
		comboBox_->signalEdited().connect(this, &PropertyRowWidgetStringListValue::onChange);
	}
	PropertyRowWidgetStringListValue(PropertyRowStringListStaticValue* row, PropertyTreeModel* model)
	: PropertyRowWidget(row, model)
	, comboBox_(new AutoDropComboBox())
	{
		comboBox_->set(StringListValue(row->value()), false);
		comboBox_->signalEdited().connect(this, &PropertyRowWidgetStringListValue::onChange);
	}

	~PropertyRowWidgetStringListValue()
	{
	}

	void onChange(){
		if ( PropertyRowStringListValue* row = dynamic_cast<PropertyRowStringListValue*>(this->row()) )
		{
            model()->push(row);
			StringListValue comboList;
			comboBox_->get(comboList);
			row->setValue(comboList);
			model()->rowChanged(row);
		}
		else if ( PropertyRowStringListStaticValue* row = dynamic_cast<PropertyRowStringListStaticValue*>(this->row()) )
		{
            model()->push(row);
			StringListStaticValue comboList = row->value();
			comboBox_->get(comboList);
			row->setValue(comboList);
			model()->rowChanged(row);
		}
	}
	void commit(){}
	Widget* actualWidget() { return comboBox_; }
protected:
	SharedPtr<ComboBox> comboBox_;
};

PropertyRowStringListValue::PropertyRowStringListValue(const char* name, const char* label, const StringListValue& value)
: PropertyRowImpl<StringListValue, PropertyRowStringListValue>(name, label, value)
{
}

PropertyRowStringListValue::PropertyRowStringListValue(void* object, size_t size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<StringListValue, PropertyRowStringListValue>(object, size, name, label, typeName)
{
}

PropertyRowWidget* PropertyRowStringListValue::createWidget(PropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree->model());
}

// ---------------------------------------------------------------------------
REGISTER_PROPERTY_ROW(StringListStaticValue, PropertyRowStringListStaticValue)

PropertyRowStringListStaticValue::PropertyRowStringListStaticValue(const char* name, const char* label, const StringListStaticValue& value)
: PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>(name, label, value)
{
}

PropertyRowStringListStaticValue::PropertyRowStringListStaticValue(void* object, size_t size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>(object, size, name, label, typeName)
{
}

PropertyRowWidget* PropertyRowStringListStaticValue::createWidget(PropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree->model());
}

}
// vim:ts=4 sw=4:
