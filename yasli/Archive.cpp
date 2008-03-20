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
