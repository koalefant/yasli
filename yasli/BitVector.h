/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

namespace yasli{

class Archive;
template<class Enum>
class BitVector
{
public:
    BitVector(int value = 0) : value_(value) {}

	operator int&() { return value_; }
    operator int() const { return value_; }

    BitVector& operator|= (Enum value) { value_ |= value; return *this; }
    BitVector& operator|= (int value) { value_ |= value; return *this; }
    BitVector& operator&= (int value) { value_ &= value; return *this; }

    void serialize(Archive& ar);
private:
    int value_;
};

}

template<class Enum>
bool serialize(yasli::Archive& ar, yasli::BitVector<Enum>& value, const char* name, const char* label);
