/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "yasli/STL.h"
#include "PropertyRowField.h"
#include "Serialization.h"

struct Unspecified_Derived_Argument {};

template<class Type, class Derived>
class PropertyRowImpl;

template<class Type, class Derived = PropertyRowImpl<Type, Unspecified_Derived_Argument> >
class PropertyRowImpl : public PropertyRowField{
public:
	bool assignTo(void* object, size_t size){
		*reinterpret_cast<Type*>(object) = value();
		return true;
	}
	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	void setValue(const Type& value) { value_ = value; }
	Type& value() { return value_; }
	const Type& value() const{ return value_; }

	void setValue(const Serializer& ser) override {
		YASLI_ESCAPE(ser.size() == sizeof(Type), return);
		value_ = *(Type*)(ser.pointer());
	}

	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
	}
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	PropertyRow* cloneSelf() const{
		Derived* result = new Derived();
		result->setNames(name_, label_, typeName_);
		result->value_ = value_;
		return result;
	}
protected:
	Type value_; 
};

