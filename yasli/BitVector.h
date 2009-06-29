#pragma once

namespace yasli{

class Archive;
template<class Enum>
class BitVector
{
public:
    BitVector(int value = 0) : value_(value) {}

    operator int&() { return value_; }

    BitVector& operator|= (Enum value) { value_ |= value; return *this; }
    BitVector& operator|= (int value) { value_ |= value; return *this; }
    BitVector& operator&= (int value) { value_ &= value; return *this; }

    void serialize(Archive& ar);
private:
    int value_;
};

template<class Enum>
bool serialize(Archive& ar, BitVector<Enum>& value, const char* name, const char* label);

}
