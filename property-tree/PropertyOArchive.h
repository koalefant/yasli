#pragma once

#include "yasli/Archive.h"

class PropertyItem;
class PropertyTreeRoot;
class PropertyItemElement;

class PropertyOArchive : public Archive{
public:
    PropertyOArchive(PropertyTreeRoot& root, PropertyItemElement* element = 0);

    // from Archive
    bool operator()(bool& value, const char* name);
    bool operator()(std::string& value, const char* name);
    bool operator()(float& value, const char* name);
    bool operator()(int& value, const char* name);
    bool operator()(long& value, const char* name);

    bool operator()(const Serializer& ser, const char* name);
    bool operator()(const ContainerSerializationInterface& ser, const char* name);
    // ^^^

    using Archive::operator();
protected:
	PropertyItem* add(PropertyItem* newItem, PropertyItem* previousItem);
    PropertyItem* rootItem();
	int containerIndex_;

    PropertyItem* currentItem_;
    PropertyItem* lastItem_;
    PropertyTreeRoot& root_;
    PropertyItemElement* element_;
};
