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

class PropertyRowStringListValue : public PropertyRowImpl<StringListValue, PropertyRowStringListValue>{
public:
	enum { Custom = true };
	PropertyRowStringListValue(const char* name = "", const char* nameAlt = "", const StringListValue& value = StringListValue());
	PropertyRowStringListValue(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName); // понадобился из за PropertyRowImpl

	// virtuals:
	PropertyRowWidget* createWidget(PropertyTree* tree);
	string valueAsString() const { return value_.c_str(); }
	bool assignTo(void* object, size_t size){
		*reinterpret_cast<StringListValue*>(object) = value().c_str();
		return true;
	}
	int widgetSizeMin() const{ return userWidgetSize() >= 0 ? userWidgetSize() : 80; }
};

class PropertyRowStringListStaticValue : public PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>{
public:
	enum { Custom = false };
	PropertyRowStringListStaticValue(const char* name = "", const char* nameAlt = "", const StringListStaticValue& value = StringListStaticValue());
	PropertyRowStringListStaticValue(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName); // понадобился из за PropertyRowImpl

	// virtuals:
	PropertyRowWidget* createWidget(PropertyTree* tree);
	string valueAsString() const { return value_.c_str(); }
	bool assignTo(void* object, size_t size){
		*reinterpret_cast<StringListStaticValue*>(object) = value().index();
		return true;
	}
	int widgetSizeMin() const{ return userWidgetSize() >= 0 ? userWidgetSize() : 80; }
};

}

