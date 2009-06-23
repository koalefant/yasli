#include "StdAfx.h"

#include "yasli/EnumDescription.h"
#include "yasli/Archive.h"
#include "yasli/MemoryWriter.h"

void EnumDescription::add(int value, const char* name, const char *label)
{
    nameToValue_[name] = value;
    labelToValue_[label] = value;
    valueToName_[value] = name;
    valueToLabel_[value] = label;
    valueToIndex_[value] = names_.size();
    names_.push_back(name);
	labels_.push_back(label);
}

bool EnumDescription::serialize(Archive& ar, int& value, const char* name, const char* label) const
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
    ar(stringListValue, name, label);
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
const char* EnumDescription::label(int value) const
{
  // TODO
  return "";
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
    CHECK(it != nameToValue_.end(), return 0);
    return it->second;
}
int EnumDescription::valueByLabel(const char* label) const
{
    LabelToValue::const_iterator it = labelToValue_.find(label);
    CHECK(it != labelToValue_.end(), return 0);
    return it->second;
}

std::string EnumDescription::labelCombination(int value) const
{
	ASSERT(0); // TODO
	return std::string();
}

