#pragma once

#include "ww/API.h"
#include "yasli/Archive.h"

namespace yasli{ class EnumDescription; }

namespace ww{

class PropertyRow;
class PropertyTreeModel;

class PropertyOArchive : public yasli::Archive{
public:
	PropertyOArchive(PropertyTreeModel* model);
	~PropertyOArchive();

protected:
	PropertyOArchive(PropertyTreeModel* model, const char* typeName, const char* derivedTypeName, const char* derivedTypeNameAlt);

	bool operator()(std::string& value, const char* name, const char* label);
	bool operator()(std::wstring& value, const char* name, const char* label);
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
	bool operator()(const PointerSerializationInterface& ptr, const char *name, const char *label);
	bool operator()(ContainerSerializationInterface& ser, const char *name, const char *label);

    bool openBlock(const char* name, const char* label);
    void closeBlock();

private:
	Archive* openDefaultArchive(const char* typeName, const char* derivedTypeName, const char* derivedTypeNameAlt);
    void closeDefaultArchive(SharedPtr<Archive> base_ar, const char* typeName, const char* derivedTypeName, const char* derivedTypeNameAlt);
	bool needDefaultArchive(const char* baseName) const { return true; }

	PropertyRow* addRow(SharedPtr<PropertyRow> newRow, bool block = false, PropertyRow* previousNode = 0);
	void enterNode(PropertyRow* row); // sets currentNode
    void closeStruct(const char* name);
	PropertyRow* rootNode();

	bool updateMode_;
	PropertyTreeModel* model_;
	SharedPtr<PropertyRow> currentNode_;
	PropertyRow* lastNode_;

	// для defaultArchive
	SharedPtr<PropertyRow> rootNode_;
	std::string typeName_;
	const char* derivedTypeName_;
	std::string derivedTypeNameAlt_;
};

}

