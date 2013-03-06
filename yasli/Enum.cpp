/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "yasli/STL.h"
#include "Enum.h"
#include "yasli/Archive.h"
#include "StringList.h"
#include "yasli/STLImpl.h"

namespace yasli{

void EnumDescription::add(int value, const char* name, const char *label)
{
	YASLI_ESCAPE( name && label, return );
	nameToValue_[name] = value;
	labelToValue_[label] = value;
	valueToName_[value] = name;
	valueToLabel_[value] = label;
	valueToIndex_[value] = int(names_.size());
	names_.push_back(name);
	labels_.push_back(label);
}

bool EnumDescription::serialize(Archive& ar, int& value, const char* name, const char* label) const
{
	if (!ar.isInPlace())
	{
		int index = StringListStatic::npos;
		if(ar.isOutput()){
			index =  indexByValue(value);
			if(index == StringListStatic::npos){
				ar.warning("Unregistered Enum value!");
			}
		}

		StringListStaticValue stringListValue(ar.isEdit() ? labels() : names(), index);
		ar(stringListValue, name, label);
		if(ar.isInput()){
			if(stringListValue.index() == StringListStatic::npos)
				return false;
			value = ar.isEdit() ? valueByLabel(stringListValue.c_str()) : this->value(stringListValue.c_str());
		}
	}
	else
	{
		return ar(value, name, label);
	}
	return true;
}

bool EnumDescription::serializeBitVector(Archive& ar, int& value, const char* name, const char* label) const
{
    if(ar.isOutput())
    {
        StringListStatic names = nameCombination(value);
		std::string str;
        joinStringList(&str, names, '|');
        return ar(str, name, label);
    }
    else
    {
		std::string str;
        if(!ar(str, name, label))
            return false;
        StringList values;
        splitStringList(&values, str.c_str(), '|');
        StringList::iterator it;
        value = 0;
        for(it = values.begin(); it != values.end(); ++it)
			if(!it->empty())
				value |= this->value(it->c_str());
		return true;
    }
}


const char* EnumDescription::name(int value) const
{
    ValueToName::const_iterator it = valueToName_.find(value);
    YASLI_ESCAPE(it != valueToName_.end(), return "");
    return it->second;
}
const char* EnumDescription::label(int value) const
{
    ValueToLabel::const_iterator it = valueToLabel_.find(value);
    YASLI_ESCAPE(it != valueToLabel_.end(), return "");
    return it->second;
}

StringListStatic EnumDescription::nameCombination(int bitVector) const 
{
    StringListStatic strings;
    for(ValueToName::const_iterator i = valueToName_.begin(); i != valueToName_.end(); ++i)
        if((bitVector & i->first) == i->first){
            bitVector &= ~i->first;
            strings.push_back(i->second);
        }
	YASLI_ASSERT(!bitVector && "Unregistered enum value");
    return strings;
}

StringListStatic EnumDescription::labelCombination(int bitVector) const 
{
    StringListStatic strings;
    for(ValueToLabel::const_iterator i = valueToLabel_.begin(); i != valueToLabel_.end(); ++i)
        if(i->second && (bitVector & i->first) == i->first){
            bitVector &= ~i->first;
            strings.push_back(i->second);
        }
	YASLI_ASSERT(!bitVector && "Unregistered enum value");
	return strings;
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
    YASLI_ESCAPE(it != nameToValue_.end(), return 0);
    return it->second;
}
int EnumDescription::valueByLabel(const char* label) const
{
    LabelToValue::const_iterator it = labelToValue_.find(label);
    YASLI_ESCAPE(it != labelToValue_.end(), return 0);
    return it->second;
}

}
// vim:ts=4 sw=4: