#include "StdAfx.h"
#include "yasli/StringList.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

StringList StringList::EMPTY;
StringListStatic StringListStatic::EMPTY;

// ---------------------------------------------------------------------------
bool serialize(Archive& ar, StringListValue& value, const char* name, const char* label)
{
    if(ar.isEdit()){
        return ar(Serializer(value), name, label);
    }
    else{
        std::string str;
        if(ar.isOutput())
            str = value.c_str();
        if(ar(str, name) && ar.isInput()){
            value = str.c_str();
            return true;
        }
        return false;
    }
}

// ---------------------------------------------------------------------------
bool serialize(Archive& ar, StringListStaticValue& value, const char* name, const char* label)
{
    if(ar.isEdit())
        return ar(Serializer(value), name, label);
    else{
        std::string str;
        if(ar.isOutput())
            str = value.c_str();
        if(ar(str, name, label) && ar.isInput()){
            value = str.c_str();
            return true;
        }
        return true;
    }
}
// ---------------------------------------------------------------------------
void splitStringList(StringList* result, const char *str, char sep)
{
  ASSERT(0);
}

bool serialize(Archive& ar, StringList& value, const char* name, const char* label)
{
	return ar(static_cast<std::vector<std::string>&>(value), name, label);
}
