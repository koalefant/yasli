/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/API.h"
#include "ww/Strings.h"
#include "yasli/Archive.h"

namespace yasli{ class EnumDescription; }

namespace ww{

class PropertyRow;
class PropertyTreeModel;

class PropertyOArchive : public yasli::Archive{
public:
	PropertyOArchive(PropertyTreeModel* model, PropertyRow* rootNode = 0);
	~PropertyOArchive();

	bool operator()(StringInterface& value, const char* name, const char* label);
	bool operator()(WStringInterface& value, const char* name, const char* label);
	bool operator()(bool& value, const char* name, const char* label);
	bool operator()(char& value, const char* name, const char* label);

	bool operator()(signed char& value, const char* name, const char* label);
	bool operator()(signed short& value, const char* name, const char* label);
	bool operator()(signed int& value, const char* name, const char* label);
	bool operator()(signed long& value, const char* name, const char* label);
	bool operator()(long long& value, const char* name, const char* label);

	bool operator()(unsigned char& value, const char* name, const char* label);
	bool operator()(unsigned short& value, const char* name, const char* label);
	bool operator()(unsigned int& value, const char* name, const char* label);
	bool operator()(unsigned long& value, const char* name, const char* label);
	bool operator()(unsigned long long& value, const char* name, const char* label);

	bool operator()(float& value, const char* name, const char* label);
	bool operator()(double& value, const char* name, const char* label);

	bool operator()(const Serializer& ser, const char* name, const char* label);
	bool operator()(PointerInterface& ptr, const char *name, const char *label);
	bool operator()(ContainerInterface& ser, const char *name, const char *label);
	bool operator()(Object& obj, const char *name, const char *label);

	bool openBlock(const char* name, const char* label);
	void closeBlock();

protected:
	PropertyOArchive(PropertyTreeModel* model, bool forDefaultType);

private:
	PropertyRow* addRow(SharedPtr<PropertyRow> newRow, bool block = false, PropertyRow* previousNode = 0);
	void enterNode(PropertyRow* row); // sets currentNode
	void closeStruct(const char* name);
	PropertyRow* rootNode();

	bool updateMode_;
	bool defaultValueCreationMode_;
	PropertyTreeModel* model_;
	SharedPtr<PropertyRow> currentNode_;
	SharedPtr<PropertyRow> lastNode_;

	// для defaultArchive
	SharedPtr<PropertyRow> rootNode_;
	string typeName_;
	const char* derivedTypeName_;
	string derivedTypeNameAlt_;
};

}

