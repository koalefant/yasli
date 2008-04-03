#pragma once
#include "PropertyItem.h"
#include "yasli/Factory.h"
#include "yasli/TypeID.h"

class Serializer;
typedef Constructor2<PropertyItem, const char*, const Serializer&> PropertyItemArgs;
typedef Factory<TypeID, PropertyItem, PropertyItemArgs> PropertyItemFactory;

#define REGISTER_PROPERTY_ITEM(dataType, itemType) \
    REGISTER_IN_FACTORY(PropertyItemFactory, TypeID::get<dataType>(), itemType) \
	SERIALIZATION_DERIVED_TYPE(PropertyItem, itemType, #dataType)
