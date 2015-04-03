/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Strings.h"
#include "ww/API.h"
#include "ww/PropertyRowImpl.h"

namespace yasli{
class EnumDescription;
}

namespace ww{

class PropertyTreeModel;

class PropertyRowStringListValue : public PropertyRowField{
public:
	enum { Custom = true };
	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }

	// virtuals:
	PropertyRowWidget* createWidget(PropertyTree* tree);
	string valueAsString() const { return value_; }
	void setValue(const Serializer& ser) override {
		const StringListValue* value = ser.cast<StringListValue>();
		stringList_ = value->stringList();
		value_ = value->c_str();
		index_ = value->index();
	}
	bool assignTo(void* object, size_t size){
		StringListValue* value = reinterpret_cast<StringListValue*>(object);
		if (value->stringList().size() < (size_t)index_ &&
			value->stringList()[index_] == value_)
			*value = index_;
		else
			*value = value_.c_str();
		return true;
	}
	int widgetSizeMin() const{ return userWidgetSize() >= 0 ? userWidgetSize() : 80; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	void serializeValue(Archive& ar){
		ar(index_, "index", "Index");
		ar(value_, "value", "Value");
	}
	StringListValue value() const { return StringListValue(stringList_, index_); }
	void setValue(const StringListValue& value) { index_ = value.index(); value_ = value.c_str(); }

	PropertyRow* clone() const {
		PropertyRowStringListValue* result = new PropertyRowStringListValue();
		result->setNames(name_, label_, typeName_);
		result->value_ = value_;
		result->index_ = index_;
		result->stringList_ = stringList_;
		return cloneChildren(result, static_cast<const PropertyRowStringListValue* const>(this));
	}
private:
	int index_;
	StringList stringList_;
	string value_;
};

class PropertyRowStringListStaticValue : public PropertyRowField{
public:
	enum { Custom = false };
	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }

	// virtuals:
	PropertyRowWidget* createWidget(PropertyTree* tree);
	string valueAsString() const { return value_; }
	void setValue(const Serializer& ser) override {
		const StringListStaticValue* value = ser.cast<StringListStaticValue>();
		value_ = value->c_str();
		index_ = value->index();
		stringList_ = value->stringList();
	}
	bool assignTo(void* object, size_t size){
		StringListStaticValue* value = reinterpret_cast<StringListStaticValue*>(object);
		if (value->stringList().size() < (size_t)index_ &&
			value->stringList()[index_] == value_)
			*value = index_;
		else {
			*value = value->stringList().find(value_.c_str());
		}
		return true;
	}
	int widgetSizeMin() const{ return userWidgetSize() >= 0 ? userWidgetSize() : 80; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	void serializeValue(Archive& ar){
		ar(index_, "index", "Index");
		ar(value_, "value", "Value");
	}
	StringListValue value() const { return StringListValue(stringList_, index_); }
	void setValue(const StringListValue& value) { index_ = value.index(); value_ = value.c_str(); }

	PropertyRow* clone() const {
		PropertyRowStringListStaticValue* result = new PropertyRowStringListStaticValue();
		result->setNames(name_, label_, typeName_);
		result->value_ = value_;
		result->index_ = index_;
		result->stringList_ = stringList_;
		return cloneChildren(result, static_cast<const PropertyRowStringListStaticValue* const>(this));
	}
private:
	int index_;
	StringList stringList_;
	string value_;
};

}

