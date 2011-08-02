/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "yasli/TypesFactory.h"
#include "yasli/Archive.h"
#include "yasli/MemoryWriter.h"

namespace yasli{

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
    if(!typeID)
		return 0;
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
	else
		name_ = name;
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
    else if(typeInfo_)
        return typeInfo_->name();
	else
		return name_.c_str();
}

const char* TypeID::label() const{
	const TypeDescription* description = TypeLibrary::the().find(*this);
	if(description)
		return description->label();
	else if(typeInfo_)
		return typeInfo_->name();
	else
		return name_.c_str();
}

}

bool serialize(yasli::Archive& ar, yasli::TypeID& typeID, const char* name, const char* label)
{
	std::string typeName;
	if(ar.isOutput()){
		typeName = typeID.name();
		return ar(typeName, name);
	}
	else{
		if(ar(typeName, name)){
			if(!typeName.empty())
				typeID = yasli::TypeID(typeName.c_str());
			else
				typeID = yasli::TypeID();
			if(!typeID){
				yasli::MemoryWriter msg;
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

