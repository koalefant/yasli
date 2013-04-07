/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
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
#include <math.h>

template<class T>
yasli::string numberAsString(T value)
{
	yasli::MemoryWriter buf;
	buf << value;
	return buf.c_str();
}

inline long long stringToSignedInteger(const char* str)
{
	long long value;
#ifdef _MSC_VER
	value = _atoi64(str);
#else
    char* endptr = (char*)str;
    value = strtoll(str, &endptr, 10);
#endif
	return value;
}

inline unsigned long long stringToUnsignedInteger(const char* str)
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
        char* endptr = (char*)str;
		value = strtoull(str, &endptr, 10);
#endif
	}
	return value;
}

template<class Output, class Input>
Output clamp(Input value, Output min, Output max)
{
	if (value < Input(min))
		return min;
	if (value > Input(max))
		return max;
	return Output(value);
}

template<class T> void clampToType(char* out, T value) { *out = clamp(value, CHAR_MIN, CHAR_MAX); }
template<class T> void clampToType(signed char* out, T value) { *out = clamp(value, SCHAR_MIN, SCHAR_MAX); }
template<class T> void clampToType(short* out, T value) { *out = clamp(value, SHRT_MIN, SHRT_MAX); }
template<class T> void clampToType(int* out, T value) { *out = clamp(value, INT_MIN, INT_MAX); }
template<class T> void clampToType(long* out, T value) { *out = clamp(value, LONG_MIN, LONG_MAX); }
template<class T> void clampToType(long long* out, T value) { *out = clamp(value, LLONG_MIN, LLONG_MAX); }

template<class T> void clampToType(unsigned char* out, T value) { *out = clamp(value, 0, UCHAR_MAX); }
template<class T> void clampToType(unsigned short* out, T value) { *out = clamp(value, 0, USHRT_MAX); }
template<class T> void clampToType(unsigned int* out, T value) { *out = clamp(value, (unsigned int)0, UINT_MAX); }
template<class T> void clampToType(unsigned long* out, T value) { *out = clamp(value, (unsigned long)0, ULONG_MAX); }
template<class T> void clampToType(unsigned long long* out, T value) { *out = clamp(value, (unsigned long long)0, ULLONG_MAX); }

template<class T> void clampToType(float* out, T value) { *out = clamp(value, -FLT_MAX, FLT_MAX); }
template<class T> void clampToType(double* out, T value) { *out = clamp(value, -DBL_MAX, DBL_MAX); }

inline void clampedNumberFromString(char* value, const char* str)        { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(signed char* value, const char* str) { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(short* value, const char* str)		 { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(int* value, const char* str)         { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(long* value, const char* str)		 { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(long long* value, const char* str)   { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(unsigned char* value, const char* str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned short* value, const char* str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned int* value, const char* str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned long* value, const char* str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned long long* value, const char* str) { clampToType(value, stringToUnsignedInteger(str)); }
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
	void setValue(Type value)
	{
		value_ = value;
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
		PropertyRowNumber* result = new PropertyRowNumber();
		result->setNames(name_, label_, typeName_);
		result->value_ = value_;
		return cloneChildren(result, this);
	}

	void startIncrement() override
	{
		incrementStartValue_ = value_;
	}

	void endIncrement(QPropertyTree* tree) override
	{
		if (value_ != incrementStartValue_) {
			Type value = value_;
			value_ = incrementStartValue_;
			tree->model()->push(this);
			value_ = value;
			tree->model()->rowChanged(this, false);
		}
	}

	void incrementLog(float screenFraction)
	{
		if (TypeID::get<Type>() == TypeID::get<float>() || TypeID::get<Type>() == TypeID::get<double>()) 
		{
			double startPower = log10(fabs(double(incrementStartValue_) + 1.0)) - 3.0;
			double power = startPower + fabs(screenFraction) * 10.0f;
			double delta = powf(10.0f, power) - powf(10.0f, startPower) + 10.0f * fabsf(screenFraction);
			double newValue;
			if (screenFraction > 0.0f)
				newValue = double(incrementStartValue_) + delta;
			else
				newValue = double(incrementStartValue_) - delta;
#ifdef _MSC_VER
            if (_isnan(newValue)) {
#else
            if (isnan(newValue)) {
#endif
				if (screenFraction > 0.0f)
					newValue = DBL_MAX;
				else
					newValue = -DBL_MAX;
			}
			clampToType(&value_, newValue);
		}
		else
		{
			double startPower = log10(fabs(double(incrementStartValue_) + 1.0)) - 3.0;
			double power = startPower + fabs(screenFraction) * 10.0f;
			double delta = powf(10.0f, power) - powf(10.0f, startPower) + 1000.0f * fabsf(screenFraction);
			double newValue;
			if (screenFraction > 0.0f)
				newValue = double(incrementStartValue_) + delta;
			else
				newValue = double(incrementStartValue_) - delta;
#ifdef _MSC_VER
			if (_isnan(newValue)) {
#else
            if (isnan(newValue)) {
#endif
				if (screenFraction > 0.0f)
					newValue = DBL_MAX;
				else
					newValue = -DBL_MAX;
			}
			clampToType(&value_, newValue);
		}
	}
protected:
	Type incrementStartValue_; 
	Type value_; 
};
