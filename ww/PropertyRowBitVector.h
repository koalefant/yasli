/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "yasli/BitVectorImpl.h"
#include "ww/PropertyRowImpl.h"

namespace ww{

class PropertyRowBitVector : public PropertyRowImpl<BitVectorWrapper, PropertyRowBitVector> {
public:
	enum{ Custom = true };
	PropertyRowBitVector(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	PropertyRowBitVector(const char* name = "", const char* label = "", const BitVectorWrapper& wrapper = BitVectorWrapper());
	const char* value() const{ return value_.c_str(); }
	const char* valueAlt() const{ return valueAlt_.c_str(); }
	void setValueAlt(const char* value);
	int flags() const { return flags_; }
	void serializeValue(Archive& ar);
	bool assignTo(void* object, size_t size);
	string valueAsString() const{ return valueAlt_; }
	//bool isStatic() const{ return false; }
	const EnumDescription* description() { return description_; }
	PropertyRowWidget* createWidget(PropertyTree* tree);

	PropertyRow* clone() const{
		return new PropertyRowBitVector(name_, label_, BitVectorWrapper(const_cast<int*>(&flags_), description_));
	}
protected:
	string value_;
	string valueAlt_;
	int flags_;
	const EnumDescription* description_;
};

}
