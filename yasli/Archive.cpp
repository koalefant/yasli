#include "StdAfx.h"
#include "yasli/Archive.h"

#include <iostream>

void Archive::warning(const char* message)
{
    //std::cout << "WARNING, Archive: " << message << std::endl;
}

bool Archive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
    return false;
}

bool Archive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
	return (*this)(Serializer(const_cast<PointerSerializationInterface&>(ptr)), name, label);
}
// ---------------------------------------------------------------------------

bool Serializer::operator()(Archive& ar) const{
	CHECK(serializeFunc_ && object_, return false);
    return serializeFunc_(object_, ar);
}
bool Serializer::operator()(Archive& ar, const char* name, const char* label) const{
    return ar(*this, name, label);
}

// ---------------------------------------------------------------------------

void PointerSerializationInterface::serialize(Archive& ar) const
{
    TypeID baseTypeID = baseType();
    TypeID oldTypeID = type();

    if(ar.isOutput()){
        if(oldTypeID){
            if(ar(oldTypeID, "")){
                ar(serializer(), "");
            }
            else
                ar.warning("Unable to write typeID!");
        }
    }
    else{
        TypeID typeID;
        if(!ar(typeID, "")){
            if(oldTypeID){
                create(TypeID()); // 0
            }
            return;
        }

        if(oldTypeID && (!typeID || (typeID != oldTypeID)))
            create(TypeID()); // 0

        if(typeID){
            if(!get())
                create(typeID);
            ar(serializer(), "");
        }
    }	
}
