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
	PropertyIArchive(PropertyTreeModel* model);

protected:
	bool operator()(std::string& value, const char* name, const char* label);
	bool operator()(std::wstring& value, const char* name, const char* label);
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
    bool operator()(const PointerSerializationInterface& ser, const char* name, const char* label);
    bool operator()(ContainerSerializationInterface& ser, const char* name, const char* label);



    bool openBlock(const char* name, const char* label);
    void closeBlock();

	bool needDefaultArchive(const char* baseName) const { return false; }
private:
	bool openRow(const char* name, const char* label, const char* typeName);
	void closeRow(const char* name);

	PropertyTreeModel* model_;
	PropertyRow* currentNode_;
	PropertyRow* lastNode_;
};

}

