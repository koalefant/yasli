/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
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

namespace ww{

class PropertyRow;
class PropertyTreeModel;

class PropertyIArchive : public yasli::Archive{
public:
	PropertyIArchive(PropertyTreeModel* model, PropertyRow* root = 0);

	bool operator()(StringInterface& value, const char* name, const char* label);
	bool operator()(WStringInterface& value, const char* name, const char* label);
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

	bool operator()(const Serializer& ser, const char* name, const char* label);
	bool operator()(PointerInterface& ser, const char* name, const char* label);
	bool operator()(ContainerInterface& ser, const char* name, const char* label);
	bool operator()(Object& obj, const char* name, const char* label);

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
	Level* currentLevel_;

	PropertyTreeModel* model_;
	PropertyRow* currentNode_;
	PropertyRow* lastNode_;
	PropertyRow* root_;
};

}

