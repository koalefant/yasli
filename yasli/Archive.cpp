#include "StdAfx.h"
#include "yasli/Archive.h"
#include <string>
#include <iostream>

#include "yasli/TypesFactory.h"

#include "pugixml.hpp"

namespace yasli{

void Archive::warning(const char* message)
{
}

bool Archive::operator()(std::wstring& value, const char* name, const char* label)
{
	std::string str;
    if(isOutput())
        str = pugi::as_utf8(value.c_str());
    if(!(*this)(str, name, label))
        return false;
    if(isInput())
        value = pugi::as_utf16(str.c_str());
    return true;
}

bool Archive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
    return false;
}

bool Archive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
	return (*this)(Serializer(const_cast<PointerSerializationInterface&>(ptr)), name, label);
}

void Archive::notImplemented()
{
    ASSERT(0 && "Not implemented!");
}
// ---------------------------------------------------------------------------

bool Serializer::operator()(Archive& ar) const{
	ESCAPE(serializeFunc_ && object_, return false);
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

}

namespace std{
YASLI_TYPE_NAME(string, "string")
}
