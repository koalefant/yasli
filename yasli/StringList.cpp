#include "StdAfx.h"
#include "yasli/Archive.h"
#include "yasli/StringList.h"

bool serialize(Archive& ar, StringListValue& value, const char* name)
{
    if(ar.isEdit()){
        return ar(Serializer(value), name);
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
bool serialize(Archive& ar, StringListStaticValue& value, const char* name)
{
    if(ar.isEdit())
        return ar(Serializer(value), name);
    else{
        std::string str;
        if(ar.isOutput())
            str = value.c_str();
        if(ar(str, name) && ar.isInput()){
            value = str.c_str();
            return true;
        }
        return true;
    }
}
