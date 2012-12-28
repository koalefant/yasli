/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <math.h>
#include <memory>

#include "PropertyOArchive.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTree.h"

#include "ww/PropertyRowBool.h"
#include "ww/PropertyRowContainer.h"
#include "ww/PropertyRowPointer.h"
#include "ww/PropertyRowNumeric.h"
#include "ww/PropertyRowString.h"
#include "ww/PropertyRowStringListValue.h"

#include "yasli/ClassFactory.h"
#include "gdiplusUtils.h"

namespace ww{

void setUpdatedRecurse(PropertyRow* row, bool updated)
{
	row->setUpdated(updated);
	PropertyRow::iterator it;
	FOR_EACH(*row, it){
		PropertyRow* row = static_cast<PropertyRow*>(&**it);
		setUpdatedRecurse(row, updated);
	}
}

PropertyOArchive::PropertyOArchive(PropertyTreeModel* model, PropertyRow* root)
: Archive(OUTPUT | EDIT)
, model_(model)
, currentNode_(root)
, lastNode_(0)
, updateMode_(false)
, defaultValueCreationMode_(false)
, rootNode_(root)
{
	YASLI_ASSERT(model != 0);
	if(!rootNode()->empty()){
		updateMode_ = true;
		setUpdatedRecurse(rootNode(), false);
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
}

PropertyOArchive::~PropertyOArchive()
{
}

PropertyRow* PropertyOArchive::rootNode()
{
	if(rootNode_)
		return rootNode_;
	else{
		YASLI_ASSERT(model_);
		YASLI_ASSERT(model_->root());
		return model_->root();
	}
}

void PropertyOArchive::enterNode(PropertyRow* row)
{
	currentNode_ = row;
}

void PropertyOArchive::closeStruct(const char* name)
{
	if(currentNode_){
		lastNode_ = currentNode_;
		currentNode_ = currentNode_->parent();
		if(lastNode_ && updateMode_){
			PropertyRow::iterator it;
			for(it = lastNode_->begin(); it != lastNode_->end();){
				PropertyRow* row = static_cast<PropertyRow*>(&**it);
				if(!row->updated())
					it = lastNode_->erase(it);
				else
					++it;
			}
		}
		lastNode_->setLabelChanged();
	}
}

PropertyRow* PropertyOArchive::addRow(SharedPtr<PropertyRow> newRow, bool block, PropertyRow* previousNode)
{
    YASLI_ESCAPE(newRow, return 0);
	const char* label = newRow->label();
	if(!previousNode) // FIXME перенести в место вызова
		previousNode = lastNode_;

	PropertyRow* result = 0;
	if(currentNode_ == 0){
		if(updateMode_){
			newRow->swapChildren(rootNode());
			newRow->setParent(0);
				model_->setRoot(newRow);
			newRow->setUpdated(true);
			newRow->setLabel(label);
			result = newRow;
		}
		else{
			if(defaultValueCreationMode_)
				rootNode_ = newRow;
			else
				model_->setRoot(newRow);
			result = newRow;
		}
		return result;
	}
	else{
		if(updateMode_ || block){
			PropertyRow* row = currentNode_->find(newRow->name(), 0, newRow->typeName(), !block);

			// we need this to preserve order
			if(row && previousNode && previousNode->parent() == currentNode_){
				if(currentNode_->childIndex(row) != currentNode_->childIndex(previousNode) + 1){
					//newRow = row;
					currentNode_->erase(row);
					row = 0;
				}
			}

			if(row){
				currentNode_->replaceAndPreserveState(row, newRow);
				newRow->setUpdated(true);
				YASLI_ASSERT(newRow->parent() == currentNode_);
				result = newRow;
				result->setLabel(label);
			}
			else{
				if(model_->expandLevels() != 0){
					if(model_->expandLevels() == -1 ||
						 model_->expandLevels() >= currentNode_->level())
						newRow->_setExpanded(true);
				}
				result = currentNode_->add(&*newRow, previousNode);
				result->setLabel(label);
				newRow->setUpdated(true);
			}
		}
		else{
            if(model_->expandLevels() != 0){
                if(model_->expandLevels() == -1 ||
                   model_->expandLevels() >= currentNode_->level())
                    newRow->_setExpanded(true);
            }
			result = currentNode_->add(&*newRow);
		}
		return result;
	}
}

bool PropertyOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
    const char* typeName = ser.type().name();
    size_t size = ser.size();

	PropertyRowFactory& factory = PropertyRowFactory::the();
	SharedPtr<PropertyRow> row = factory.create(typeName, PropertyRowArg(ser.pointer(), size, name, label, typeName));
 	if(!row)
		row = new PropertyRow(name, label, ser);
	
	if(!row->isLeaf() || currentNode_ == 0){
		SharedPtr<PropertyRow> previousRow = lastNode_;
		lastNode_ = currentNode_;
		enterNode(addRow(row, false, previousRow));

		if(currentNode_->isLeaf())
			return false;
	}
	else{
		lastNode_ = addRow(row);
		return true;
	}

	if (ser)
		ser(*this);

    closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(StringInterface& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowString(name, label, value.get()));
	return true;
}

bool PropertyOArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowString(name, label, value.get()));
	return true;
}

bool PropertyOArchive::operator()(bool& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowBool(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(char& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<char>(name, label, value));
	return true;
}

// ---

bool PropertyOArchive::operator()(signed char& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed char>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(signed short& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed short>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(signed int& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed int>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(signed long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed long>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(long long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<long long>(name, label, value));
	return true;
}

// ---

bool PropertyOArchive::operator()(unsigned char& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned char>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned short& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned short>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned int& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned int>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned long>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned long long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned long long>(name, label, value));
	return true;
}

// ---

bool PropertyOArchive::operator()(float& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<float>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(double& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<double>(name, label, value));
	return true;
}


bool PropertyOArchive::operator()(ContainerInterface& ser, const char *name, const char *label)
{
	const char* elementTypeName = ser.type().name();
	bool fixedSizeContainer = ser.isFixedSize();
	PropertyRow* container = new PropertyRowContainer(name, label, ser.type().name(), elementTypeName, fixedSizeContainer);
	lastNode_ = currentNode_;
	enterNode(addRow(container, false));

	if (!model_->defaultTypeRegistered(elementTypeName)) {
		PropertyOArchive ar(model_, true);
		ar.setFilter(getFilter());
		model_->addDefaultType(0, elementTypeName); // add empty default to prevent recursion
		ser.serializeNewElement(ar, "default", "[+]");
		if (ar.rootNode() != 0)
			model_->addDefaultType(ar.rootNode(), ser.type().name());
	}

	if ( ser.size() > 0 )
		while( true )
		{
			ser(*this, "", "<");
			if ( !ser.next() )
				break;
		}

	closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(PointerInterface& ptr, const char *name, const char *label)
{
	lastNode_ = currentNode_;
	PropertyRow* row = new PropertyRowPointer(name, label, ptr);
	enterNode(addRow(row));

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
			const TypeDescription *desc = factory->descriptionByIndex((int)i);
			if (!model_->defaultTypeRegistered(baseType, desc->typeID())){
				PropertyOArchive ar(model_, true);
				//ar.setInnerContext(getInnerContext());
				ar.setContextMap(contextMap());
				ar.setFilter(getFilter());

				PropertyDefaultTypeValue defaultValue;
				defaultValue.type = desc->typeID();
				defaultValue.factory = factory;
				defaultValue.factoryIndex = int(i);
				defaultValue.label = desc->label();

				model_->addDefaultType(baseType, defaultValue);
				factory->serializeNewByIndex(ar, (int)i, "name", "label");
				if (ar.rootNode() != 0) {
					ar.rootNode()->setTypeName(desc->name());
					defaultValue.root = ar.rootNode();
					model_->addDefaultType(baseType, defaultValue);
				}
			}
		}
	}

	if(Serializer ser = ptr.serializer())
		ser(*this);
	currentNode_->setLabelChanged();
	closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(Object& obj, const char *name, const char *label)
{
	const char* typeName = obj.type().name();

	PropertyRowObject* row = 0;
	if (typeName_.empty())
		 row = new PropertyRowObject(name, label, obj, model_);
	else
		 row = new PropertyRowObject(name, label, Object(0, obj.type(), 0, 0, 0), model_);
	lastNode_ = addRow(row);
	return true;
}

bool PropertyOArchive::openBlock(const char* name, const char* label)
{
	PropertyRow* row = new PropertyRow(label, label, "");
	lastNode_ = currentNode_;
	enterNode(addRow(row, true));
	return true;
}

void PropertyOArchive::closeBlock()
{
	closeStruct("block");
}

}
// vim:ts=4 sw=4:
