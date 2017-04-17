/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


#pragma once
#include "PropertyRowImpl.h"
#include "PropertyTreeModel.h"
#include "IDrawContext.h"
#include "PropertyTreeBase.h"
#include "IUIFacade.h"
#include "Rect.h"
#include <vector>


using yasli::StringListValue;
class PropertyRowStringListValue : public PropertyRow, public ComboBoxClientRow {
public:
	property_tree::InplaceWidget* createWidget(PropertyTreeBase* tree) override;

	void populateComboBox(std::vector<std::string>* values, int* selectedIndex, PropertyTreeBase* tree) override
	{
		values->assign(stringList_.begin(), stringList_.end());
		*selectedIndex = stringList_.find(value_.c_str());
	}

	bool onComboBoxSelected(const char* newValue, PropertyTreeBase* tree) override
	{
		if (value_ != newValue) {
			tree->model()->rowAboutToBeChanged(this);
			value_ = newValue;
			tree->model()->rowChanged(this);
			return true;
		}
		return false;
	}

	yasli::string valueAsString() const override { return value_.c_str(); }
	bool assignTo(const Serializer& ser) const override {
		*ser.cast<StringListValue>() = value_.c_str();
		return true;
	}
	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		YASLI_ESCAPE(ser.size() == sizeof(StringListValue), return);
		const StringListValue& stringListValue = *ser.cast<StringListValue>();
		stringList_ = stringListValue.stringList();
		value_ = stringListValue.c_str();
		handle_ = stringListValue.handle();
		type_ = stringListValue.type();
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	int widgetSizeMin(const PropertyTreeBase* tree) const override
	{
		if (userWidgetToContent())
			return widthCache_.getOrUpdate(tree, this, tree->_defaultRowHeight());
		else
			return 80;
	}
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	const void* searchHandle() const override { return handle_; }
	yasli::TypeID searchType() const override { return type_; }
	yasli::TypeID typeId() const override{ return type_; }

	void redraw(IDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(context.widgetRect, " ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(inlinedSelected(), valueAsString().c_str());
		else
			context.drawComboBox(context.widgetRect, valueAsString().c_str());

	}

	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	const void* handle_;
	yasli::TypeID type_;
	mutable RowWidthCache widthCache_;
	friend class InplaceWidgetStringListValue;
};

using yasli::StringListStaticValue;
class PropertyRowStringListStaticValue : public PropertyRowImpl<StringListStaticValue>, public ComboBoxClientRow {
public:
	InplaceWidget* createWidget(PropertyTreeBase* tree) override;

	void populateComboBox(std::vector<std::string>* values, int* selectedIndex, PropertyTreeBase* tree) override
	{
		values->assign(stringList_.begin(), stringList_.end());
		*selectedIndex = stringList_.find(value_.c_str());
	}

	bool onComboBoxSelected(const char* newValue, PropertyTreeBase* tree) override
	{
		if (value_ != newValue) {
			tree->model()->rowAboutToBeChanged(this);
			value_ = newValue;
			tree->model()->rowChanged(this);
			return true;
		}
		return false;
	}
	yasli::string valueAsString() const override { return value_.c_str(); }
	bool assignTo(const Serializer& ser) const override {
		*ser.cast<StringListStaticValue>() = value_.c_str();
		return true;
	}
	void setValueAndContext(const Serializer& ser, yasli::Archive& ar) override {
		const StringListStaticValue& stringListValue = *ser.cast<StringListStaticValue>();
		stringList_.resize(stringListValue.stringList().size());
		for (size_t i = 0; i < stringList_.size(); ++i)
			stringList_[i] = stringListValue.stringList()[i];
		value_ = stringListValue.c_str();
		handle_ = stringListValue.handle();
		type_ = stringListValue.type();
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	int widgetSizeMin(const PropertyTreeBase* tree) const override
	{
		if (userWidgetToContent())
			return widthCache_.getOrUpdate(tree, this, tree->_defaultRowHeight());
		else
			return 80;
	}
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	const void* searchHandle() const override { return handle_; }
	yasli::TypeID searchType() const override { return type_; }
	yasli::TypeID typeId() const override{ return type_; }
	void redraw(IDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(context.widgetRect, " ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(inlinedSelected(), valueAsString().c_str());
		else
			context.drawComboBox(context.widgetRect, valueAsString().c_str());
	}


	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	friend class InplaceWidgetStringListValue;
	const void* handle_;
	yasli::TypeID type_;
	mutable RowWidthCache widthCache_;
	friend class PropertyRowWidgetStringListValue;
};
	
