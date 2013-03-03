/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "QPropertyTree.h"
#include "PropertyRowImpl.h"
#include "yasli/MemoryWriter.h"

#include <QtGui/QLineEdit>

class PropertyRowNumericInterface
{
public:
	virtual bool setValueFromString(const char* str) = 0;
	virtual yasli::string valueAsString() const = 0;
};


class PropertyRowWidgetNumeric : public PropertyRowWidget
{
	Q_OBJECT
public:
   PropertyRowWidgetNumeric(PropertyRow* row, PropertyTreeModel* mode, PropertyRowNumericInterface * numeric, QPropertyTree* tree);
	~PropertyRowWidgetNumeric(){
		if (entry_)
			entry_->setParent(0);
		entry_->deleteLater();
		entry_ = 0;
	}

	void commit();

	QWidget* actualWidget() { return entry_; }
public slots:
	void onEditingFinished();
protected:
	QLineEdit* entry_;
	PropertyRowNumericInterface* numeric_;
	QPropertyTree* tree_;
};


template<class T>
yasli::string numericAsString(T value)
{
	yasli::MemoryWriter buf;
	buf << value;
	return buf.c_str();
}

template<class Type>
class PropertyRowNumber : public PropertyRowField, public PropertyRowNumericInterface{
public:
	enum { Custom = false };
	PropertyRowNumber()
		: PropertyRowField("", "", TypeID::get<Type>().name())
	{
	}
	PropertyRowNumber(const char* name, const char* nameAlt, Type value)
	: PropertyRowField(name, nameAlt, TypeID::get<Type>().name())
	, value_(value)
	{
	}

	PropertyRowWidget* createWidget(QPropertyTree* tree){
		return new PropertyRowWidgetNumeric(this, tree->model(), this, tree);
	}

	int widgetSizeMin() const{ 
        if (PropertyRow::userWidgetSize() >= 0)
            return PropertyRow::userWidgetSize();
		else
			return 40;
	}

	bool setValueFromString(const char* str) override{
        Type value = value_;
        value_ = Type(atof(str));
        return value_ != value;
	}
	yasli::string valueAsString() const override{ 
        return numericAsString(Type(value_));
	}

	bool assignTo(void* object, size_t size){
		*reinterpret_cast<Type*>(object) = value_;
		return true;
	}
	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }

	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
	}
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	PropertyRow* clone() const{
		return cloneChildren(new PropertyRowNumber(name_, label_, value_), this);
	}
protected:
	Type value_; 
};
