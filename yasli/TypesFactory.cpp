#include "StdAfx.h"

#include "yasli/TypesFactory.h"
#include "yasli/Archive.h"
#include "utils/MemoryWriter.h"


TypeLibrary& TypeLibrary::the()
{
    static TypeLibrary typeLibrary;
    return typeLibrary;
}

TypeLibrary::TypeLibrary()
{

}

const TypeDescription* TypeLibrary::find(TypeID typeID) const
{
    ASSERT(typeID);
    TypeToDescriptionMap::const_iterator it = typeToDescriptionMap_.find(typeID);
    if(it != typeToDescriptionMap_.end())
        return it->second;
    else
        return 0;
}

const TypeDescription* TypeLibrary::findByName(const char* name) const
{
    ASSERT(name && strlen(name));
    TypeToDescriptionMap::const_iterator it;

    for(it = typeToDescriptionMap_.begin(); it != typeToDescriptionMap_.end(); ++it)
        if(strcmp(it->second->name(), name) == 0)
            return it->second;

    return 0;
}

const TypeDescription* TypeLibrary::registerType(const TypeDescription* description){
    typeToDescriptionMap_[description->typeID()] = description;
    return description;
}

// ----------------------------------------------------------------------------

TypeID TypeID::ZERO;

TypeID::TypeID(const char* name)
: typeInfo_(0)
{
    const TypeDescription* description = TypeLibrary::the().findByName(name);
    if(description)
        *this = description->typeID();
}

bool TypeID::registered() const{
    return TypeLibrary::the().find(*this) != 0;
}

std::size_t TypeID::sizeOf() const{
    const TypeDescription* description = TypeLibrary::the().find(*this);
    if(description)
        return description->size();
    else
        return 0;
}

const char* TypeID::name() const{
    const TypeDescription* description = TypeLibrary::the().find(*this);
    if(description)
        return description->name();
    else
        return "";
}

bool serialize(Archive& ar, TypeID& typeID, const char* name)
{
    std::string typeName;
    if(ar.isOutput()){
        typeName = typeID.name();
        if(typeName == ""){
            MemoryWriter msg;
            if(typeID.typeInfo())
                msg << "Unable to write TypeID, unregistered type_info: '" << typeID.typeInfo()->name() << "'";
            else
                msg << "Unable to write TypeID, null type_info.";
            ar.warning(msg.c_str());
            return false;
        }
        else
            return ar(typeName, name);
    }
    else{
        if(ar(typeName, name)){
            typeID = TypeID(typeName.c_str());
            if(!typeID){
                MemoryWriter msg;
                msg << "Unable to read TypeID, unregistered type name: \'" << typeName.c_str() << "'";
                ar.warning(msg.c_str());
                return false;
            }
            else
                return true;
        }
        else
            return false;
    }
}
