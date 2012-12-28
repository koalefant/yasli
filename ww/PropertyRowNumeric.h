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
#include "ww/Entry.h"
#include "yasli/MemoryWriter.h"

namespace yasli{
class EnumDescription;
}

namespace ww{

class PropertyTreeModel;

// используется widget-ом редактриования
class PropertyRowNumericInterface{
public:
	virtual bool setValueFromString(const char* str) = 0;
	virtual string valueAsString() const = 0;
};


class PropertyRowWidgetNumeric : public PropertyRowWidget, public has_slots{
public:
    PropertyRowWidgetNumeric(PropertyRow* row, PropertyTreeModel* mode, PropertyRowNumericInterface * numeric, PropertyTree* tree);
	~PropertyRowWidgetNumeric(){}

	void onChange();
	void commit();

	Widget* actualWidget() { return entry_; }
protected:
	SharedPtr<Entry> entry_;
	PropertyRowNumericInterface* numeric_;
	PropertyTree* tree_;
};


template<class T>
string numericAsString(T value)
{
	MemoryWriter buf;
	buf << value;
	return buf.c_str();
}



template<class Derived, class Default>
struct SelectNumericDerived{
	typedef Derived Type;
};

template<class Default>
struct SelectNumericDerived<Unspecified_Derived_Argument, Default>{
	typedef Default Type;
};

template<class Type, class _Derived = Unspecified_Derived_Argument >
class PropertyRowNumeric : public PropertyRowImpl<Type, typename SelectNumericDerived<_Derived, PropertyRowNumeric<Type, _Derived> >::Type>, public PropertyRowNumericInterface{
public:
	enum { Custom = false };
	typedef typename SelectNumericDerived<_Derived, PropertyRowNumeric>::Type Derived;
	PropertyRowNumeric(const char* name = "", const char* nameAlt = "", Type value = Type())
	: PropertyRowImpl<Type, Derived>((void*)(&value), sizeof(Type), name, nameAlt, TypeID::get<Type>().name())
	{
	}
	PropertyRowNumeric(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
	: PropertyRowImpl<Type, Derived>(object, size, name, nameAlt, typeName)
	{
	}
	PropertyRowWidget* createWidget(PropertyTree* tree){
		return new PropertyRowWidgetNumeric(this, tree->model(), this, tree);
	}

	int widgetSizeMin() const{ 
		if (userWidgetSize() >= 0)
			return userWidgetSize();
		else
			return 40;
	}

	bool setValueFromString(const char* str){
		Type value = value_;
		value_ = Type(atof(str));
		return value_ != value;
	}
	string valueAsString() const{ 
		return numericAsString(Type(value_)); 
	}
};

}

