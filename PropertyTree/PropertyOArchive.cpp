/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include <math.h>
#include <memory>

#include "yasli/STL.h"
#include "PropertyOArchive.h"
#include "PropertyTreeModel.h"
#include "PropertyTree.h"

#include "PropertyRowContainer.h"
#include "PropertyRowBool.h"
#include "PropertyRowString.h"
#include "PropertyRowNumber.h"
#include "PropertyRowPointer.h"
#include "PropertyRowObject.h"
#include "ConstStringList.h"

#include "yasli/Archive.h"
#include "yasli/BitVector.h"
#include "yasli/ClassFactory.h"
#include "yasli/StringList.h"
using yasli::TypeID;


PropertyOArchive::PropertyOArchive(PropertyTreeModel* model, PropertyRowStruct* root)
: Archive(OUTPUT | EDIT)
, model_(model)
, currentNode_(root)
, lastNode_(0)
, updateMode_(false)
, defaultValueCreationMode_(false)
, rootNode_(root)
{
	stack_.push_back(Level());
	YASLI_ASSERT(model != 0);
	if(!rootNode_->empty()){
		updateMode_ = true;
		stack_.back().oldRows.swap(rootNode_->children_);
	}
}



PropertyOArchive::PropertyOArchive(PropertyTreeModel* model, bool forDefaultType)
: Archive(OUTPUT | EDIT)
, model_(model)
, currentNode_(0)
, lastNode_(0)
, updateMode_(false)
, defaultValueCreationMode_(forDefaultType)
, rootNode_(0)
{
	rootNode_ = new PropertyRowStruct();
	rootNode_->setName("root");
	currentNode_ = rootNode_.get();
	stack_.push_back(Level());
}

PropertyOArchive::~PropertyOArchive()
{
}

PropertyRow* PropertyOArchive::defaultValueRootNode()
{
	if (!rootNode_)
		return 0;
	return rootNode_->childByIndex(0);
}

void PropertyOArchive::enterNode(PropertyRowStruct* row)
{
	if (!row->isStruct()) {
		YASLI_ASSERT(0);
		return;
	}
	currentNode_ = row;

	stack_.push_back(Level());
	Level& level = stack_.back();
	level.oldRows.swap(row->children_);
	row->children_.reserve(level.oldRows.size());	
}

void PropertyOArchive::closeStruct(const char* name)
{
	stack_.pop_back();

	if(currentNode_){
		lastNode_ = currentNode_;
		currentNode_ = currentNode_->parent();
	}
}

static PropertyRow* findRow(int* index, PropertyRows& rows, const char* name, const char* typeName, int startIndex)
{
	int count = int(rows.size());
	int iterations = 1;
	for(int i = startIndex; i < count; ++i){
		PropertyRow* row = rows[i];
		if (!row)
			continue;
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
		   (row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0)) {
			*index = (int)i;
			if (iterations > 1)
				printf("Searching %d times for \"%s\"\n", iterations, name);
			return row;
		}
		++iterations;
	}
	for(int i = 0; i < startIndex; ++i){
		PropertyRow* row = rows[i];
		if (!row)
			continue;
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
		   (row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0)) {
			*index = (int)i;
			if (iterations > 1)
				printf("Searching %d times for \"%s\"\n", iterations, name);
			return row;
		}
		++iterations;
	}
	return 0;
}

template<class RowType, class ValueType>
RowType* PropertyOArchive::updateRow(const char* name, const char* label, const char* typeName, const ValueType& value)
{
	SharedPtr<RowType> newRow;
	if(currentNode_ == 0){
		if (rootNode_)
			newRow = static_cast<RowType*>(rootNode_.get());
		else		
			newRow.reset(new RowType());
		newRow->setNames(name, label, typeName);
		if(updateMode_){
			model_->setRoot(newRow);
			return newRow;
		}
		else{
			if(defaultValueCreationMode_)
				rootNode_ = newRow;
			else
				model_->setRoot(newRow);
			newRow->setValueAndContext(value, *this);
			return newRow;
		}
	}
	else{

		Level& level = stack_.back();
		int rowIndex;
		PropertyRow* oldRow = findRow(&rowIndex, level.oldRows, name, typeName, level.rowIndex);

		const char* oldLabel = 0;
		if(oldRow){
			oldRow->setMultiValue(false);
			oldLabel = oldRow->label();
			newRow = static_cast<RowType*>(oldRow);
			level.oldRows[rowIndex] = 0;
			level.rowIndex = rowIndex + 1;
		}
		else{
			//printf("creating new row '%s' '%s' '%s'\n", name, label, typeName);
			PropertyRowFactory& factory = PropertyRowFactory::the();
			newRow = static_cast<RowType*>(factory.create(typeName));
			if(!newRow)
				newRow.reset(new RowType());

			if(model_->expandLevels() != 0 && (model_->expandLevels() == -1 || model_->expandLevels() >= currentNode_->level()))
				newRow->_setExpanded(true);
		}
		newRow->setNames(name, label, typeName);
		currentNode_->add(newRow);
		if (!oldRow || oldLabel != label) {
			// for new rows we should mark all parents with labelChanged_
			newRow->setLabelChanged();
			newRow->setLabelChangedToChildren();
		}
		newRow->setValueAndContext(value, *this);
		return newRow;
	}
}

template<class RowType, class ValueType>
PropertyRow* PropertyOArchive::updateRowPrimitive(const char* name, const char* label, const char* typeName, const ValueType& value)
{
	SharedPtr<RowType> newRow;

	if(currentNode_ == 0)
		return 0;

	int rowIndex;
	Level& level = stack_.back();
	PropertyRow* oldRow = findRow(&rowIndex, level.oldRows, name, typeName, level.rowIndex);

	const char* oldLabel = 0;
	if(oldRow){
		oldRow->setMultiValue(false);
		newRow.reset(static_cast<RowType*>(oldRow));
		level.oldRows[rowIndex] = 0;
		level.rowIndex = rowIndex + 1;
		oldLabel = oldRow->label();
		oldRow->setNames(name, label, typeName);
	}
	else{
		//printf("creating new row '%s' '%s' '%s'\n", name, label, typeName);
		newRow = new RowType();
		newRow->setNames(name, label, typeName);
		if(model_->expandLevels() != 0){
			if(model_->expandLevels() == -1 || model_->expandLevels() >= currentNode_->level())
				newRow->_setExpanded(true);
		}
	}
	currentNode_->add(newRow);
	if (!oldRow || oldLabel != label)
		// for new rows we should mark all parents with labelChanged_
		newRow->setLabelChanged();

	newRow->setValue(value);
	return newRow;
}

bool PropertyOArchive::operator()(const yasli::Serializer& ser, const char* name, const char* label)
{
    const char* typeName = ser.type().name();
    size_t size = ser.size();

	lastNode_ = currentNode_;
	PropertyRowStruct* row = updateRow<PropertyRowStruct>(name, label, typeName, ser);
	PropertyRow* nonLeaf = 0;
	if(!row->isLeaf() || currentNode_ == 0){
		enterNode(row);

		if(currentNode_->isLeaf())
			return false;
		else
			nonLeaf = currentNode_;
	}
	else{
		lastNode_ = row;
		return true;
	}

	if (ser)
		ser(*this);

	if (nonLeaf)
		nonLeaf->closeNonLeaf(ser);

    closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(yasli::StringInterface& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowString>(name, label, "string", value.get());
	return true;
}

bool PropertyOArchive::operator()(yasli::WStringInterface& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowString>(name, label, "string", value.get());
	return true;
}

bool PropertyOArchive::operator()(bool& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowBool>(name, label, "bool", value);
	return true;
}

bool PropertyOArchive::operator()(char& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<char> >(name, label, "char", value);
	return true;
}

// ---

bool PropertyOArchive::operator()(signed char& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<signed char> >(name, label, "signed char", value);
	return true;
}

bool PropertyOArchive::operator()(signed short& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<signed short> >(name, label, "short", value);
	return true;
}

bool PropertyOArchive::operator()(signed int& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<signed int> >(name, label, "int", value);
	return true;
}

bool PropertyOArchive::operator()(signed long& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<signed long> >(name, label, "long", value);
	return true;
}

bool PropertyOArchive::operator()(long long& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<long long> >(name, label, "long long", value);
	return true;
}

// ---

bool PropertyOArchive::operator()(unsigned char& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<unsigned char> >(name, label, "unsigned char", value);
	return true;
}

bool PropertyOArchive::operator()(unsigned short& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<unsigned short> >(name, label, "unsigned short", value);
	return true;
}

bool PropertyOArchive::operator()(unsigned int& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<unsigned int> >(name, label, "unsigned int", value);
	return true;
}

bool PropertyOArchive::operator()(unsigned long& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<unsigned long> >(name, label, "unsigned long", value);
	return true;
}

bool PropertyOArchive::operator()(unsigned long long& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<unsigned long long> >(name, label, "unsigned long long", value);
	return true;
}

// ---

bool PropertyOArchive::operator()(float& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<float> >(name, label, "float", value);
	return true;
}

bool PropertyOArchive::operator()(double& value, const char* name, const char* label)
{
	lastNode_ = updateRowPrimitive<PropertyRowNumber<double> >(name, label, "double", value);
	return true;
}


bool PropertyOArchive::operator()(yasli::ContainerInterface& ser, const char *name, const char *label)
{
	const char* elementTypeName = ser.elementType().name();
	bool fixedSizeContainer = ser.isFixedSize();
	lastNode_ = currentNode_;
	enterNode(updateRow<PropertyRowContainer>(name, label, ser.containerType().name(), ser));

	if (!model_->defaultTypeRegistered(elementTypeName)) {
		PropertyOArchive ar(model_, true);
		ar.setFilter(getFilter());
		ar.setLastContext(lastContext());
		model_->addDefaultType(0, elementTypeName); // add empty default to prevent recursion
		ser.serializeNewElement(ar, "", "<");
		if (ar.defaultValueRootNode() != 0)
			model_->addDefaultType(ar.defaultValueRootNode(), elementTypeName);
	}
	if ( ser.size() > 0 )
		while( true ) {
			ser(*this, "", "<");
			if ( !ser.next() )
				break;
		}
	currentNode_->labelChanged();
	closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(yasli::PointerInterface& ptr, const char *name, const char *label)
{
	lastNode_ = currentNode_;
	PropertyRowPointer* row = updateRow<PropertyRowPointer>(name, label, ptr.baseType().name(), ptr);
	enterNode(row);
	{
		TypeID baseType = ptr.baseType();
		yasli::ClassFactoryBase* factory = ptr.factory();
		size_t count = factory->size();

		const char* nullLabel = factory->nullLabel();
		if (!(nullLabel && nullLabel[0] == '\0'))
		{
			PropertyDefaultTypeValue nullValue;
			nullValue.type = TypeID();
			nullValue.factory = factory;
			nullValue.factoryIndex = -1;
			nullValue.label = nullLabel ? nullLabel : "[ null ]";
			model_->addDefaultType(baseType, nullValue);
		}

		for(size_t i = 0; i < count; ++i) {
			const yasli::TypeDescription *desc = factory->descriptionByIndex((int)i);
			if (!model_->defaultTypeRegistered(baseType, desc->typeID())){
				PropertyOArchive ar(model_, true);
				ar.setLastContext(lastContext());
				ar.setFilter(getFilter());

				PropertyDefaultTypeValue defaultValue;
				defaultValue.type = desc->typeID();
				defaultValue.factory = factory;
				defaultValue.factoryIndex = int(i);
				defaultValue.label = desc->label();

				model_->addDefaultType(baseType, defaultValue);
				factory->serializeNewByIndex(ar, (int)i, "name", "label");
				if (ar.defaultValueRootNode() != 0) {
					ar.defaultValueRootNode()->setTypeName(desc->name());
					defaultValue.root = ar.defaultValueRootNode();
					model_->addDefaultType(baseType, defaultValue);
				}
			}
		}
	}

	if(Serializer ser = ptr.serializer())
		ser(*this);
	closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(yasli::Object& obj, const char *name, const char *label)
{
	const char* typeName = obj.type().name();

	PropertyRowObject* row = 0;
	if (typeName_.empty())
		 row = updateRow<PropertyRowObject>(name, label, obj.type().name(), obj);
	else
		 row = updateRow<PropertyRowObject>(name, label, obj.type().name(), obj);
	lastNode_ = row;
	return true;
}

bool PropertyOArchive::openBlock(const char* name, const char* label)
{
	PropertyRowStruct* row = updateRow<PropertyRowStruct>(name, label, "block", Serializer());
	lastNode_ = currentNode_;
	enterNode(row);
	return true;
}

void PropertyOArchive::closeBlock()
{
	closeStruct("block");
}

// vim:ts=4 sw=4:
