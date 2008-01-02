#pragma once

#include "yasli/Archive.h"

class PropertyItem;
class PropertyTreeRoot;

class PropertyOArchive : public Archive{
public:
    PropertyOArchive(PropertyTreeRoot& root);

    // from Archive
    bool operator()(bool& value, const char* name);
    bool operator()(std::string& value, const char* name);
    bool operator()(float& value, const char* name);
    bool operator()(int& value, const char* name);
    bool operator()(long& value, const char* name);

    bool operator()(const Serializer& ser, const char* name);
    bool operator()(const ContainerSerializer& ser, const char* name);
    // ^^^

    template<class T>
    bool operator()(const T& value, const char* name){
        return Archive::operator()(value, name);
	}
protected:
	PropertyItem* add(PropertyItem* newItem, PropertyItem* previousItem);

    PropertyItem* currentItem_;
    PropertyItem* lastItem_;
    PropertyTreeRoot& root_;
};
