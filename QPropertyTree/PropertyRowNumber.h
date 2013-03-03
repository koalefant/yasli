/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "QPropertyTree.h"
#include "yasli/MemoryWriter.h"
#include "PropertyRowNumberField.h"

#include <limits.h>
#include <float.h>

template<class T>
yasli::string numberAsString(T value)
{
	yasli::MemoryWriter buf;
	buf << value;
	return buf.c_str();
}

template<class T>
void stringToSignedInteger(T* out, const char* str, T min, T max)
{
	long long value;
#ifdef _MSC_VER
	value = _atoi64(str);
#else
	char** endptr = str;
	value = strtoll(str, &str, 10);
#endif
	if (value < min)
		value = min;
	if (value > max)
		value = max;
	*out = value;
}

template<class T>
void stringToUnsignedInteger(T* out, const char* str, T min, T max)
{
	unsigned long long value;
	if (*str == '-') {
		value = 0;
	}
	else {
#ifdef _MSC_VER
		char* endptr = (char*)str;
		value = _strtoui64(str, &endptr, 10);
#else
		char* endptr = str;
		value = strtoull(str, &endptr, 10);
#endif
	}
	if (value < min)
		value = min;
	if (value > max)
		value = max;
	*out = value;
}

inline void clampedNumberFromString(char* value, const char* str) { stringToSignedInteger<char>(value, str, CHAR_MIN, CHAR_MAX); }
inline void clampedNumberFromString(signed char* value, const char* str) { stringToSignedInteger<signed char>(value, str, SCHAR_MIN, SCHAR_MAX); }
inline void clampedNumberFromString(short* value, const char* str) { stringToSignedInteger<short>(value, str, SHRT_MIN, SHRT_MAX); }
inline void clampedNumberFromString(int* value, const char* str) { stringToSignedInteger<int>(value, str, INT_MIN, INT_MAX); }
inline void clampedNumberFromString(long* value, const char* str) { stringToSignedInteger<long>(value, str, LONG_MIN, LONG_MAX); }
inline void clampedNumberFromString(long long* value, const char* str) { stringToSignedInteger<long long>(value, str, LLONG_MIN, LLONG_MAX); }
inline void clampedNumberFromString(unsigned char* value, const char* str) { stringToUnsignedInteger<unsigned char>(value, str, 0, UCHAR_MAX); }
inline void clampedNumberFromString(unsigned short* value, const char* str) { stringToUnsignedInteger<unsigned short>(value, str, 0, USHRT_MAX); }
inline void clampedNumberFromString(unsigned int* value, const char* str) { stringToUnsignedInteger<unsigned int>(value, str, 0, UINT_MAX); }
inline void clampedNumberFromString(unsigned long* value, const char* str) { stringToUnsignedInteger<unsigned long>(value, str, 0, ULONG_MAX); }
inline void clampedNumberFromString(unsigned long long* value, const char* str) { stringToUnsignedInteger<unsigned long long>(value, str, 0, ULLONG_MAX); }
inline void clampedNumberFromString(float* value, const char* str)
{
	double v = atof(str);
	if (v > FLT_MAX)
		v = FLT_MAX;
	if (v < -FLT_MAX)
		v = -FLT_MAX;
	*value = float(v);
}

inline void clampedNumberFromString(double* value, const char* str)
{
	*value = atof(str);
}


template<class Type>
class PropertyRowNumber : public PropertyRowNumberField{
public:
	enum { Custom = false };
	PropertyRowNumber()
	: PropertyRowNumberField("", "", TypeID::get<Type>().name())
	{
	}
	PropertyRowNumber(const char* name, const char* nameAlt, Type value)
	: PropertyRowNumberField(name, nameAlt, TypeID::get<Type>().name())
	, value_(value)
	{
	}

	bool setValueFromString(const char* str) override{
        Type value = value_;
		clampedNumberFromString(&value_, str);
        return value_ != value;
	}
	yasli::string valueAsString() const override{ 
        return numberAsString(Type(value_));
	}

	bool assignTo(void* object, size_t size){
		*reinterpret_cast<Type*>(object) = value_;
		return true;
	}

	void serializeValue(yasli::Archive& ar){
		ar(value_, "value", "Value");
	}
	PropertyRow* clone() const{
		return cloneChildren(new PropertyRowNumber(name_, label_, value_), this);
	}
protected:
	Type value_; 
};
