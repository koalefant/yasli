/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "yasli/STL.h"
#include "ClassFactory.h"
#include "yasli/Archive.h"
#include "MemoryWriter.h"

#include "yasli/Strings.h"

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
    YASLI_ASSERT(name && strlen(name));
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

bool TypeID::registered() const{
    return TypeLibrary::the().find(*this) != 0;
}

const char* TypeID::name() const{
#if YASLI_NO_RTTI
	if (typeInfo_)
		return typeInfo_->name;
	else
		return "";
#else
    const TypeDescription* description = TypeLibrary::the().find(*this);
    if(description)
        return description->name();
    else if(typeInfo_)
        return typeInfo_->name();
	else
		return name_.c_str();
#endif
}

size_t TypeID::sizeOf() const{
#if YASLI_NO_RTTI
	if (typeInfo_)
		return typeInfo_->size;
	else
		return 0;
#else
    const TypeDescription* description = TypeLibrary::the().find(*this);
    if(description)
        return description->size();
    else
        return 0;
#endif
}

bool serialize(Archive& ar, TypeIDWithFactory& value, const char* name, const char* label)
{
	std::string typeName;
	if(ar.isOutput()){
		typeName = value.type.name();
		return ar(typeName, name);
	}
	else{
		if(ar(typeName, name)){
			if(!typeName.empty())
				value.type = value.factory->findTypeByName(typeName.c_str());
			else
				value.type = TypeID();
			if(!value.type){
				char msg[128];
#ifdef _MSC_VER
				_snprintf_s(msg, sizeof(msg), _TRUNCATE, "Unable to read TypeID: unregistered type name: \'%s\'", typeName.c_str());
#else
                snprintf(msg, sizeof(msg), "Unable to read TypeID: unregistered type name: \'%s\'", typeName.c_str());
                msg[sizeof(msg)-1] = '\0';
#endif
				ar.warning(msg);
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}
}

}
