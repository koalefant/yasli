#pragma once

#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include <memory>

namespace pugi{
    class xml_document;
}

namespace yasli{

struct XMLIArchiveImpl;
class YASLI_API XMLIArchive : public Archive{
public:
    XMLIArchive();
    ~XMLIArchive();

    void open(const char* filename);
    void openFromMemory(const char* buffer, size_t length); // does copy of buffer
    bool isOpen() const;
    void close();

    const char* pull();
    bool operator()(bool& value, const char* name = "");
    bool operator()(std::string& value, const char* name = "");
    bool operator()(float& value, const char* name = "");
    bool operator()(double& value, const char* name = "");
    bool operator()(int& value, const char* name = "");
    bool operator()(unsigned int& value, const char* name = "");
	bool operator()(long long& value, const char* name = "");

    bool operator()(signed char& value, const char* name = "");
    bool operator()(unsigned char& value, const char* name = "");
    bool operator()(char& value, const char* name = "");

    bool operator()(const Serializer& ser, const char* name = "");
    bool operator()(ContainerSerializationInterface& ser, const char* name = "");

	using Archive::operator();
private:
    bool findAttribute(const char* name);
    bool findNode(const char* name);
    bool enterNode();
    void leaveNode();

	std::auto_ptr<XMLIArchiveImpl> impl_;
    friend struct XMLIArchiveImpl;
};

}
