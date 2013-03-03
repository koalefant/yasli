/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "Serialization.h"
#include "yasli/Enum.h"
#include "PropertyTreeModel.h"
#include "PropertyIArchive.h"
#include "PropertyRowBool.h"
#include "PropertyRowString.h"
#include "PropertyRowNumber.h"
#include "PropertyRowPointer.h"
#include "PropertyRowObject.h"
#include "Unicode.h"

using yasli::TypeID;

PropertyIArchive::PropertyIArchive(PropertyTreeModel* model, PropertyRow* root)
: Archive(INPUT | EDIT)
, model_(model)
, currentNode_(0)
, lastNode_(0)
, root_(root)
{
	if (!root_)
		root_ = model_->root();
	else
		currentNode_ = root;
}

bool PropertyIArchive::operator()(yasli::StringInterface& value, const char* name, const char* label)
{
	if(openRow(name, label, "string")){
		if(PropertyRowString* row = static_cast<PropertyRowString*>(currentNode_))
 			value.set(fromWideChar(row->value().c_str()).c_str());
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(yasli::WStringInterface& value, const char* name, const char* label)
{
	if(openRow(name, label, "string")){
		if(PropertyRowString* row = static_cast<PropertyRowString*>(currentNode_)) {
			value.set(row->value().c_str());
		}
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(bool& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<bool>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(char& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<char>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

// Signed types
bool PropertyIArchive::operator()(signed char& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<signed char>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(signed short& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<signed short>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(signed int& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<signed int>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(signed long& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<signed long>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(long long& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<long long>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

// Unsigned types
bool PropertyIArchive::operator()(unsigned char& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<unsigned char>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned short& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<unsigned short>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned int& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<unsigned int>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned long& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<unsigned long>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(unsigned long long& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<unsigned long long>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(float& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<float>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(double& value, const char* name, const char* label)
{
	if(openRow(name, label, TypeID::get<double>().name())){
		currentNode_->assignTo(value);
		closeRow(name);
		return true;
	}
	else
		return false;
}

bool PropertyIArchive::operator()(yasli::ContainerInterface& ser, const char* name, const char* label)
{
    const char* typeName = ser.type().name();
	if(!openRow(name, label, typeName))
        return false;

    size_t size = 0;
	if(currentNode_->multiValue())
		size = ser.size();
	else{
		size = currentNode_->count();
		size = ser.resize(size);
	}

	size_t index = 0;
    if(ser.size() > 0)
        while(index < size)
        {
            ser(*this, "", "<");
            ser.next();
			++index;
        }

    closeRow(name);
	return true;
}

bool PropertyIArchive::operator()(const yasli::Serializer& ser, const char* name, const char* label)
{
	if(openRow(name, label, ser.type().name())){
		if(currentNode_->isLeaf() && !currentNode_->isRoot()){
            currentNode_->assignTo(ser.pointer(), ser.size());
            closeRow(name);
            return true;
		}
    }
	else
		return false;

    ser(*this);

    closeRow(name);
	return true;
}


bool PropertyIArchive::operator()(yasli::PointerInterface& ser, const char* name, const char* label)
{
	const char* baseName = ser.baseType().name();

	if(openRow(name, label, baseName)){
		if (!currentNode_->isPointer()) {
			closeRow(name);
			return false;
		}

		YASLI_ASSERT(currentNode_);
		PropertyRowPointer* row = static_cast<PropertyRowPointer*>(currentNode_);
		if(!row){
			closeRow(name);
			return false;
		}
		row->assignTo(ser);
	}
	else
		return false;

	if(ser.get() != 0)
		ser.serializer()( *this );

	closeRow(name);
	return true;
}

bool PropertyIArchive::operator()(yasli::Object& obj, const char* name, const char* label)
{
	if(openRow(name, label, obj.type().name())){
		bool result = false;
		if (currentNode_->isObject()) {
			PropertyRowObject* rowObj = static_cast<PropertyRowObject*>(currentNode_);
			result = rowObj->assignTo(&obj);
		}
		closeRow(name);
		return result;
	}
	else
		return false;
}

bool PropertyIArchive::openBlock(const char* name, const char* label)
{
	if(openRow(label, label, "")){
		return true;
	}
	else
		return false;
}

void PropertyIArchive::closeBlock()
{
	closeRow("block");
}

bool PropertyIArchive::openRow(const char* name, const char* label, const char* typeName)
{
	if(!name)
		return false;
	if(name[0] == '\0' && label && label[0] != '\0')
		name = label;

	if(!currentNode_){
		lastNode_ = currentNode_ = model_->root();
		YASLI_ASSERT(currentNode_);
		return true;
	}

	YASLI_ESCAPE(currentNode_, return false);
	
	if(currentNode_->empty())
		return false;

	PropertyRow* node = 0;
	if(currentNode_->isContainer()){
		if(lastNode_ == currentNode_){
			node = static_cast<PropertyRow*>(currentNode_->front());
		}
		else{
			PropertyRow* row = lastNode_;
			while(row != root_ && row->parent() && currentNode_ != row->parent())
				row = row->parent();
			
			PropertyRow::iterator iter = std::find(currentNode_->begin(), currentNode_->end(), row);
			if(iter != currentNode_->end()){
				++iter;

				if(iter != currentNode_->end())
					node = static_cast<PropertyRow*>(&**iter);
			}
		}
	}
	else
		node = currentNode_->find(name, 0, typeName);

	if(node){
		lastNode_ = node;
		if(node->isContainer() || !node->multiValue()){
			currentNode_ = node;
			return true;
		}
	}
	return false;
}

void PropertyIArchive::closeRow(const char* name)
{
	YASLI_ESCAPE(currentNode_, return);
	currentNode_ = currentNode_->parent();
}
