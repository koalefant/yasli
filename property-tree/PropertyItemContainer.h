#pragma once
#include "PropertyItem.h"

class ContainerSerializer;
class PropertyItemContainer : public PropertyItem{
public:
    PropertyItemContainer();
    PropertyItemContainer(const char* name, const ContainerSerializer& zer,
                          bool fixedSize);
    bool isContainer() const{ return true; }
protected:
    TypeID childrenType_;
    bool fixedSize_;
};
