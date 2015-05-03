/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "PropertyTree.h"
#include "yasli/MemoryWriter.h"
#include "yasli/decorators/Range.h"
#include "PropertyRowNumberField.h"

#include <limits>
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

template<class Out, class In> void clampToType(Out* out, In value) { *out = clamp(value, std::numeric_limits<Out>::lowest(), std::numeric_limits<Out>::max()); }

static bool isDigit(int ch) 
{
	return unsigned(ch - '0') <= 9;
}

static double parseFloat(const char* s)
{
	double res = 0, f = 1, sign = 1;
	while(*s && (*s == ' ' || *s == '\t')) 
		s++;

	if(*s == '-') 
		sign=-1, s++; 
	else if (*s == '+') 
		s++;

	for(; isDigit(*s); s++)
		res = res * 10 + (*s - '0');

	if(*s == '.' || *s == ',')
		for (s++; isDigit(*s); s++)
			res += (f *= 0.1)*(*s - '0');
	return res*sign;
}

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
	double v = parseFloat(str);
	if (v > FLT_MAX)
		v = FLT_MAX;
	if (v < -FLT_MAX)
		v = -FLT_MAX;
	*value = float(v);
}

inline void clampedNumberFromString(double* value, const char* str)
{
	*value = parseFloat(str);
}


template<class Type>
class PropertyRowNumber : public PropertyRowNumberField{
public:
	PropertyRowNumber()
	{
		softMin_ = std::numeric_limits<Type>::lowest();
		softMax_ = std::numeric_limits<Type>::max();
		hardMin_ = std::numeric_limits<Type>::lowest();
		hardMax_ = std::numeric_limits<Type>::max();
	}

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

	bool assignToPrimitive(void* object, size_t size) const override{
		*reinterpret_cast<Type*>(object) = value_;
		return true;
	}

    void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		yasli::RangeDecorator<Type>* range = (yasli::RangeDecorator<Type>*)ser.pointer();
		value_ = *range->value;
		softMin_ = range->softMin;
		softMax_ = range->softMax;
		hardMin_ = range->hardMin;
		hardMax_ = range->hardMax;
	}

    bool assignTo(const yasli::Serializer& ser) const override {
		yasli::RangeDecorator<Type>* range = (yasli::RangeDecorator<Type>*)ser.pointer();
		*range->value = value_;
        return true;
	}

	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
		ar(softMin_, "softMin", "SoftMin");
		ar(softMax_, "softMax", "SoftMax");
		ar(hardMin_, "hardMin", "HardMin");
		ar(hardMax_, "hardMax", "HardMax");
	}

	void startIncrement() override
	{
		incrementStartValue_ = value_;
	}

	void endIncrement(PropertyTree* tree) override
	{
		if (value_ != incrementStartValue_) {
			Type value = value_;
			value_ = incrementStartValue_;
			tree->model()->rowAboutToBeChanged(this);
			value_ = value;
			tree->model()->rowChanged(this, false);
		}
	}

	void incrementLog(float screenFraction) override
	{
		bool bothSoftLimitsSet = (std::numeric_limits<Type>::lowest() == 0 || softMin_ != std::numeric_limits<Type>::lowest()) && softMax_ != std::numeric_limits<Type>::max();

		if (bothSoftLimitsSet)
		{
			Type softRange = softMax_ - softMin_;
			double newValue = incrementStartValue_ + softRange * screenFraction * 3.0f;
			value_ = clamp(newValue, hardMin_, hardMax_);
		}
		else
		{
			double screenFractionMultiplier = 1000.0;
			if (yasli::TypeID::get<Type>() == yasli::TypeID::get<float>() || yasli::TypeID::get<Type>() == yasli::TypeID::get<double>()) 
				screenFractionMultiplier = 10.0;

			double startPower = log10(fabs(double(incrementStartValue_)) + 1.0) - 3.0;
			double power = startPower + fabs(screenFraction) * 10.0f;
			double delta = pow(10.0, power) - pow(10.0, startPower) + screenFractionMultiplier * fabs(screenFraction);
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
			value_ = clamp(newValue, hardMin_, hardMax_);
		}
	}
protected:
	Type incrementStartValue_; 
	Type value_; 
	Type softMin_;
	Type softMax_;
	Type hardMin_;
	Type hardMax_;
};
