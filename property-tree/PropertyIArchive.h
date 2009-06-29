#ifndef __PROPERTY_I_ARCHIVE_H_INCLUDED__
#define __PROPERTY_I_ARCHIVE_H_INCLUDED__

#include "yasli/Archive.h"
using namespace yasli;

class PropertyItem;
class PropertyTreeRoot;

class PropertyIArchive : public yasli::Archive{
public:
    PropertyIArchive(const PropertyTreeRoot& root);

    // from Archive
    bool operator()(bool& value, const char* name, const char* label);
    bool operator()(std::string& value, const char* name, const char* label);
    bool operator()(float& value, const char* name, const char* label);
    bool operator()(int& value, const char* name, const char* label);
    bool operator()(long& value, const char* name, const char* label);

    bool operator()(const Serializer& value, const char* name, const char* label);
	bool operator()(ContainerSerializationInterface& ser, const char* name, const char* label);
	bool operator()(const PointerSerializationInterface& ser, const char* name, const char* label);
    // ^^^
	
    template<class T>
    bool operator()(const T& value, const char* name){
        return Archive::operator()(value, name);
    }
protected:
	const PropertyItem* findChild(const char* name);
    const PropertyTreeRoot& root_;

    const PropertyItem* currentItem_;
    const PropertyItem* currentChild_;
};

#endif
