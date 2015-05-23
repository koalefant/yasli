/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Archive.h"

namespace yasli{
	class EnumDescription;
}

class PropertyRow;
class PropertyTreeModel;

class PropertyIArchive final : public yasli::Archive{
public:
	PropertyIArchive(PropertyTreeModel* model, PropertyRow* root = 0);

	bool operator()(yasli::StringInterface& value, const char* name, const char* label);
	bool operator()(yasli::WStringInterface& value, const char* name, const char* label);
    bool operator()(bool& value, const char* name, const char* label);
    bool operator()(char& value, const char* name, const char* label);
    
    // Signed types
    bool operator()(signed char& value, const char* name, const char* label);
    bool operator()(signed short& value, const char* name, const char* label);
    bool operator()(signed int& value, const char* name, const char* label);
    bool operator()(signed long& value, const char* name, const char* label);
    bool operator()(long long& value, const char* name, const char* label);
    // Unsigned types
    bool operator()(unsigned char& value, const char* name, const char* label);
    bool operator()(unsigned short& value, const char* name, const char* label);
    bool operator()(unsigned int& value, const char* name, const char* label);
    bool operator()(unsigned long& value, const char* name, const char* label);
    bool operator()(unsigned long long& value, const char* name, const char* label);

    bool operator()(float& value, const char* name, const char* label);
    bool operator()(double& value, const char* name, const char* label);

	bool operator()(const yasli::Serializer& ser, const char* name, const char* label);
	bool operator()(yasli::PointerInterface& ser, const char* name, const char* label);
	bool operator()(yasli::ContainerInterface& ser, const char* name, const char* label);
	bool operator()(yasli::Object& obj, const char* name, const char* label);

	bool openBlock(const char* name, const char* label);
	void closeBlock();

protected:
	bool needDefaultArchive(const char* baseName) const { return false; }
private:
	bool openRow(const char* name, const char* label, const char* typeName);
	void closeRow(const char* name);

	struct Level {
		int rowIndex;
		Level() : rowIndex(0) {}
	};

	vector<Level> stack_;

	PropertyTreeModel* model_;
	PropertyRow* currentNode_;
	PropertyRow* lastNode_;
	PropertyRow* root_;
};

