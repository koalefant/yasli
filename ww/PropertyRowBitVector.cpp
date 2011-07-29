#include "StdAfx.h"
#include "PropertyRowBitVector.h"
#include "PropertyRow.h"
#include "PropertyTree.h"
#include "ww/CheckComboBox.h"
#include "ww/PropertyTreeModel.h"
#include "ww/SafeCast.h"
#include "yasli/PointersImpl.h"

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
        model()->push(row);
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

PropertyRowBitVector::PropertyRowBitVector(const char* name, const char* label, const BitVectorWrapper& wrapper)
: PropertyRowImpl(name, label, wrapper)
, description_(wrapper.description)
, flags_(wrapper.value)
{
	if(description_){
		StringListStatic values = description_->nameCombination(flags_);
		joinStringList(&value_, values, '|');
		StringListStatic labels = description_->labelCombination(flags_);
		joinStringList(&valueAlt_, labels, '|');
	}
	else{
		typeName_ = "";
		//value_ = "";
		valueAlt_ = "";
	}
}

PropertyRowBitVector::PropertyRowBitVector(void* object, size_t _size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<BitVectorWrapper, PropertyRowBitVector>(object, _size, name, label, typeName)
, description_(0)
, flags_(0)
{
	ESCAPE(_size == sizeof(BitVectorWrapper), return);
	BitVectorWrapper* wrapper = reinterpret_cast<BitVectorWrapper*>(object);
	ESCAPE(wrapper != 0, return);
	description_ = wrapper->description;
	ESCAPE(description_ != 0, return);
	flags_ = wrapper->value;
	if(description_){
		StringListStatic values = description_->nameCombination(flags_);
		joinStringList(&value_, values, '|');
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
	ASSERT(size == sizeof(BitVectorWrapper));
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
