/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyRowBitVector.h"
#include "PropertyRow.h"
#include "PropertyTree.h"
#include "ww/CheckComboBox.h"
#include "ww/PropertyTreeModel.h"
#include "ww/SafeCast.h"
#include "yasli/PointersImpl.h"
#include "yasli/ClassFactory.h"

namespace ww{
class AutoDropCheckComboBox : public CheckComboBox{
public:
	void show(){
		CheckComboBox::show();
		showDropDown();
	}
};

class PropertyRowWidgetBitVector : public PropertyRowWidget, public has_slots{
public:
	PropertyRowWidgetBitVector(PropertyRowBitVector* row, PropertyTreeModel* model)
	: PropertyRowWidget(row, model)
	, comboBox_(new AutoDropCheckComboBox())
	{
		comboBox_->set( row->description()->labels(), row->valueAlt() );
		comboBox_->setDropDownHeight(50);
		comboBox_->signalEdited().connect(this, &PropertyRowWidgetBitVector::onChange);
		//comboBox_->showDropDown();
	}
	~PropertyRowWidgetBitVector() {}

	void onChange(){
		PropertyRowBitVector* row = safe_cast<PropertyRowBitVector*>(this->row());
        model()->rowAboutToBeChanged(row);
		row->setValueAlt(comboBox_->value());
		//row->setIndex(comboBox_->selectedIndex());
		model()->rowChanged(row);
	}
	void commit(){
	}
	Widget* actualWidget() { return comboBox_; }
protected:
	SharedPtr<CheckComboBox> comboBox_;
};

//YASLI_CLASS(PropertyRow, PropertyRowBitVector, "BitVector");

PropertyRowBitVector::PropertyRowBitVector()
: description_(BitVectorWrapper::currentDescription)
, flags_(0)
{
	YASLI_ASSERT(description_)
	
}

void PropertyRowBitVector::setValue(const BitVectorWrapper& wrapper)
{
	if(description_){
		StringListStatic values = description_->nameCombination(flags_);
		joinStringList(&valueText_, values, '|');
		StringListStatic labels = description_->labelCombination(flags_);
		joinStringList(&valueAlt_, labels, '|');
	}
	else{
		typeName_ = "";
		//value_ = "";
		valueAlt_ = "";
	}
}


bool PropertyRowBitVector::assignTo(void* object, size_t size)
{
	YASLI_ASSERT(size == sizeof(BitVectorWrapper));
	reinterpret_cast<BitVectorWrapper*>(object)->value = flags();
	return true;
}

void PropertyRowBitVector::setValueAlt(const char* value)
{
	StringList strings;
	StringList::iterator sit;
	splitStringList(&strings, value, '|');
	int flags = 0;
	for(sit = strings.begin(); sit != strings.end(); ++sit){
		if(!sit->empty())
			flags |= description_->valueByLabel(sit->c_str());
	}
	flags_ = flags;
	valueAlt_ = value;
}


PropertyRowWidget* PropertyRowBitVector::createWidget(PropertyTree* tree)
{
	return new PropertyRowWidgetBitVector(this, tree->model());
}

void PropertyRowBitVector::serializeValue(Archive& ar)
{
	ar.serialize(valueAlt_, "valueAlt", "ValueAlt");
	ar.serialize(flags_, "flags", "Flags");
}

REGISTER_PROPERTY_ROW(BitVectorWrapper, PropertyRowBitVector)
DECLARE_SEGMENT(PropertyRowBitVector)

}
