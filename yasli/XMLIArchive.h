#pragma once

#include "yasli/Pointers.h"
#include "yasli/Archive.h"

namespace pugi{
    class xml_document;
}


struct XMLIArchiveImpl;
class YASLI_API XMLIArchive : public Archive{
public:
    XMLIArchive();
    ~XMLIArchive();

    void open(const char* filename);
    void openFromMemory(const char* buffer, size_t length, bool free = false);
    bool isOpen() const;
    void close();

    const char* pull();
    bool operator()(bool& value, const char* name = "");
    bool operator()(std::string& value, const char* name = "");
    bool operator()(float& value, const char* name = "");
    bool operator()(double& value, const char* name = "");
    bool operator()(int& value, const char* name = "");
    bool operator()(unsigned int& value, const char* name = "");
	bool operator()(__int64& value, const char* name = "");

    bool operator()(signed char& value, const char* name = "");
    bool operator()(unsigned char& value, const char* name = "");
    bool operator()(char& value, const char* name = "");

    bool operator()(const Serializer& ser, const char* name = "");
    bool operator()(const ContainerSerializationInterface& ser, const char* name = "");

	using Archive::operator();
private:
    bool findAttribute(const char* name);
    bool findNode(const char* name);
    bool enterNode();
    void leaveNode();

    AutoPtr<XMLIArchiveImpl> impl_;
    friend struct XMLIArchiveImpl;
};
