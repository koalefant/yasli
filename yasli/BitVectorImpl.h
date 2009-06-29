#pragma once
#include "yasli/BitVector.h"
#include "yasli/Archive.h"

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


template<class Enum>
bool serialize(Archive& ar, BitVector<Enum>& value, const char* name, const char* label)
{
    EnumDescription &desc = getEnumDescription<Enum>();
    if(ar.isEdit())
        return ar(BitVectorWrapper(&static_cast<int&>(value), &desc), name, label);
    else
    {
        return desc.serializeBitVector(ar, static_cast<int&>(value), name, label);
    }
}

}
