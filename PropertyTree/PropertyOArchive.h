/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Strings.h"
#include "yasli/Archive.h"
#include "yasli/Pointers.h"

class PropertyRow;
class PropertyTreeModel;
using yasli::SharedPtr;

class PropertyOArchive : public yasli::Archive{
public:
	PropertyOArchive(PropertyTreeModel* model, PropertyRow* rootNode = 0);
	~PropertyOArchive();

	bool operator()(yasli::StringInterface& value, const char* name, const char* label);
	bool operator()(yasli::WStringInterface& value, const char* name, const char* label);
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

	bool operator()(const yasli::Serializer& ser, const char* name, const char* label);
	bool operator()(yasli::PointerInterface& ptr, const char *name, const char *label);
	bool operator()(yasli::ContainerInterface& ser, const char *name, const char *label);
	bool operator()(yasli::Object& obj, const char *name, const char *label);

	bool openBlock(const char* name, const char* label);
	void closeBlock();

protected:
	PropertyOArchive(PropertyTreeModel* model, bool forDefaultType);

private:
	struct Level {
		std::vector<SharedPtr<PropertyRow> > oldRows;
		int rowIndex;
		Level() : rowIndex(0) {}
	};
	std::vector<Level> stack_;

	template<class RowType, class ValueType>
	PropertyRow* updateRowPrimitive(const char* name, const char* label, const char* typeName, const ValueType& value);

	template<class RowType, class ValueType>
	RowType* updateRow(const char* name, const char* label, const char* typeName, const ValueType& value);

	void enterNode(PropertyRow* row); // sets currentNode
	void closeStruct(const char* name);
	PropertyRow* defaultValueRootNode();

	bool updateMode_;
	bool defaultValueCreationMode_;
	PropertyTreeModel* model_;
	SharedPtr<PropertyRow> currentNode_;
	SharedPtr<PropertyRow> lastNode_;

	// for defaultArchive
	SharedPtr<PropertyRow> rootNode_;
	yasli::string typeName_;
	const char* derivedTypeName_;
	yasli::string derivedTypeNameAlt_;
};

// vim:ts=4 sw=4:
