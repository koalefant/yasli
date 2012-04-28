/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "yasli/BitVector.h"
#include "yasli/Archive.h"
#include "yasli/EnumDescription.h"

namespace yasli{

struct BitVectorWrapper
{
	int* valuePointer;
	int value;
    const EnumDescription* description;

	explicit BitVectorWrapper(int* _value = 0, const EnumDescription* _description = 0)
    : valuePointer(_value)
    , description(_description)
    {
		if(valuePointer)
			value = *valuePointer;
    }
    BitVectorWrapper(const BitVectorWrapper& _rhs)
    : value(_rhs.value)
    , description(0)
	, valuePointer(0)
    {

    }

	~BitVectorWrapper()
	{
		if(valuePointer)
			*valuePointer = value;
	}
	BitVectorWrapper& operator=(const BitVectorWrapper& rhs){
		value = rhs.value;
		return *this;
	}


    void serialize(Archive& ar)
    {
		ar(value, "value", "Value");
    }
};

template<class Enum>
void BitVector<Enum>::serialize(Archive& ar)
{
    ar(value_, "value", "Value");
}

}

template<class Enum>
bool serialize(yasli::Archive& ar, yasli::BitVector<Enum>& value, const char* name, const char* label)
{
    using namespace yasli;
    EnumDescription &desc = getEnumDescription<Enum>();
    if(ar.isEdit())
        return ar(BitVectorWrapper(&static_cast<int&>(value), &desc), name, label);
    else
    {
        return desc.serializeBitVector(ar, static_cast<int&>(value), name, label);
    }
}

