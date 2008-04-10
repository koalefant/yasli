#include "StdAfx.h"
#include "yasli/Archive.h"

#include <iostream>

void Archive::warning(const char* message)
{
    //std::cout << "WARNING, Archive: " << message << std::endl;
}

bool Archive::operator()(const ContainerSerializationInterface& ser, const char* name)
{
    return false;
}

bool Archive::operator()(const PointerSerializationInterface& ptr, const char* name)
{
	return (*this)(Serializer(const_cast<PointerSerializationInterface&>(ptr)), name);
}
// ---------------------------------------------------------------------------

bool Serializer::operator()(Archive& ar) const{
    //ASSERT(serializeFunc_ && object_);
    if(serializeFunc_ && object_)
        return serializeFunc_(object_, ar);
    else
        return false;
}
bool Serializer::operator()(Archive& ar, const char* name) const{
    return ar(*this, name);
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
