#include "StdAfx.h"

#include "yasli/EnumDescription.h"
#include "yasli/Archive.h"
#include "utils/MemoryWriter.h"

void EnumDescription::add(int value, const char* name)
{
    nameToValue_[name] = value;
    valueToName_[value] = name;
    valueToIndex_[value] = names_.size();
    names_.push_back(name);
}

bool EnumDescription::serialize(Archive& ar, int& value, const char* name) const
{
    int index = StringListStatic::npos;
    if(ar.isOutput()){
        index =  indexByValue(value);
        if(index == StringListStatic::npos){
            ar.warning("Unregistered Enum value!");
            return false;
        }
    }

    StringListStaticValue stringListValue(names(), index);
    ar(stringListValue, name);
    if(ar.isInput()){
        if(stringListValue.index() == StringListStatic::npos)
            return false;
        value = this->value(stringListValue.c_str());
    }
    return true;
}
const char* EnumDescription::name(int value) const
{
    ValueToName::const_iterator it = valueToName_.find(value);
    ASSERT(it != valueToName_.end());
    return it->second;
}
int EnumDescription::indexByValue(int value) const
{
    ValueToIndex::const_iterator it = valueToIndex_.find(value);
    if(it == valueToIndex_.end())
        return -1;
    else
        return it->second;
}
int EnumDescription::value(const char* name) const
{
    NameToValue::const_iterator it = nameToValue_.find(name);
    ASSERT(it != nameToValue_.end());
    return it->second;
}
