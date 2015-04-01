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
	PropertyRowBitVector();
	const char* value() const{ return valueText_.c_str(); }
	const char* valueAlt() const{ return valueAlt_.c_str(); }
	void setValueAlt(const char* value);
	int flags() const { return flags_; }
	void serializeValue(Archive& ar);
	bool assignTo(void* object, size_t size);
	string valueAsString() const{ return valueAlt_; }
	void setValue(const Serializer& ser) override;
	//bool isStatic() const{ return false; }
	const EnumDescription* description() { return description_; }
	PropertyRowWidget* createWidget(PropertyTree* tree);

	PropertyRow* clone() const{
		PropertyRowBitVector* result = new PropertyRowBitVector();
		result->setNames(name_, label_, typeName_);
		result->valueText_ = valueText_;
		result->valueAlt_ = valueAlt_;
		result->flags_ = flags_;
		result->description_ = description_;
		return cloneChildren(result, this);
	}
protected:
	string valueText_;
	string valueAlt_;
	int flags_;
	const EnumDescription* description_;
};

}
