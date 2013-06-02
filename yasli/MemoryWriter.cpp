/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/Assert.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _MSC_VER
# include <float.h>
# define isnan _isnan
#else
# include <wchar.h>
#endif

#include "MemoryWriter.h"

namespace yasli{

MemoryWriter::MemoryWriter(size_t size, bool reallocate)
: size_(size)
, reallocate_(reallocate)
, digits_(5)
{
    alloc(size);
}

MemoryWriter::~MemoryWriter()
{
    position_ = 0;
    free(memory_);
}

void MemoryWriter::alloc(size_t initialSize)
{
    memory_ = (char*)malloc(initialSize + 1);
    position_ = memory_;
}

void MemoryWriter::realloc(size_t newSize)
{
    YASLI_ASSERT(newSize > size_);
    size_t pos = position();
    memory_ = (char*)::realloc(memory_, newSize + 1);
    YASLI_ASSERT(memory_ != 0);
    position_ = memory_ + pos;
    size_ = newSize;
}

MemoryWriter& MemoryWriter::operator<<(int value)
{
    // TODO: optimize
    char buffer[12];
    sprintf(buffer, "%i", value);
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(long value)
{
    // TODO: optimize
    char buffer[12];
#ifdef _MSC_VER
    sprintf(buffer, "%i", value);
#else
    sprintf(buffer, "%li", value);
#endif
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(unsigned long value)
{
    // TODO: optimize
    char buffer[12];
#ifdef _MSC_VER
    sprintf(buffer, "%u", value);
#else
    sprintf(buffer, "%lu", value);
#endif
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(long long value)
{
    // TODO: optimize
    char buffer[24];
#ifdef _MSC_VER
    sprintf(buffer, "%I64i", value);
#else
    sprintf(buffer, "%lli", value);
#endif
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(unsigned long long value)
{
    // TODO: optimize
    char buffer[24];
#ifdef _MSC_VER
    sprintf(buffer, "%I64u", value);
#else
    sprintf(buffer, "%llu", value);
#endif
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(unsigned int value)
{
    // TODO: optimize
    char buffer[12];
    sprintf(buffer, "%u", value);
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(char value)
{
    char buffer[12];
    sprintf(buffer, "%i", int(value));
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(unsigned char value)
{
    char buffer[12];
    sprintf(buffer, "%i", int(value));
    return operator<<((const char*)buffer);
}

MemoryWriter& MemoryWriter::operator<<(signed char value)
{
    char buffer[12];
    sprintf(buffer, "%i", int(value));
    return operator<<((const char*)buffer);
}

inline void cutRightZeros(const char* str)
{
	for(char* p = (char*)str + strlen(str) - 1; p >= str; --p)
		if(*p == '0')
			*p = 0;
		else
			return;
}

MemoryWriter& MemoryWriter::operator<<(double value)
{
#ifdef ANDROID_NDK
	char buf[64] = { 0 };
	sprintf(buf, "%f", value);
	operator<<(buf);
#else
	// YASLI_ASSERT(!isnan(value)); disabled, because physics data is not always initialized

	int point = 0;
	int sign = 0;

#ifdef _MSC_VER
	char buf[_CVTBUFSIZE];
	_fcvt_s(buf, value, digits_, &point, &sign);
#else
    const char* buf = fcvt(value, digits_, &point, &sign);
#endif

    if(sign != 0)
        write("-");
    if(point <= 0){
		cutRightZeros(buf);
		if(strlen(buf)){
	        write("0.");
			while(point < 0){
				write("0");
				++point;
			}
			write(buf);
		}
		else
			write("0");
        *position_ = '\0';
    }
    else{
        write(buf, point);
        write(".");
		cutRightZeros(buf + point);
		operator<<(buf + point);
    }
	return *this;
#endif
}

MemoryWriter& MemoryWriter::operator<<(const char* value)
{
    write((void*)value, strlen(value));
    YASLI_ASSERT(position() < size());
    *position_ = '\0';
    return *this;
}

MemoryWriter& MemoryWriter::operator<<(const wchar_t* value)
{
#if defined(ANDROID_NDK) || defined(NACL)
	return *this;
#else
    write((void*)value, wcslen(value) * sizeof(wchar_t));
    YASLI_ASSERT(position() < size());
    *position_ = '\0';
    return *this;
#endif
}

void MemoryWriter::setPosition(size_t pos)
{
    YASLI_ASSERT(pos < size_);
    YASLI_ASSERT(memory_ + pos <= position_);
    position_ = memory_ + pos;
}

void MemoryWriter::write(const char* value)
{
    write((void*)value, strlen(value));
}

bool MemoryWriter::write(const void* data, size_t size)
{
    YASLI_ASSERT(memory_ <= position_);
    YASLI_ASSERT(position() < this->size());
    if(size_ - position() > size){
        memcpy(position_, data, size);
        position_ += size;
    }
    else{
        if(!reallocate_)
            return false;

        realloc(size_ * 2);
        write(data, size);
    }
    YASLI_ASSERT(position() < this->size());
    return true;
}

void MemoryWriter::write(char c)
{
    if(size_ - position() > 1){
        *(char*)(position_) = c;
        ++position_;
    }
    else{
		YASLI_ESCAPE(reallocate_, return);
        realloc(size_ * 2);
        write(c);
    }
    YASLI_ASSERT(position() < this->size());
}

}
