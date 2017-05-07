/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#if !YALSI_NO_RTTI
#include <typeinfo>
#endif
#include "yasli/Config.h"
#include "yasli/Assert.h"
#include <string.h>

namespace yasli{

#if !YASLI_NO_RTTI
#ifndef _MSC_VER
using std::type_info;
#endif
#endif

class Archive;
struct TypeInfo;

class TypeID{
	friend class TypesFactory;
public:
	TypeID()
	: module_(0)
	, typeInfo_(0)
	{}

	TypeID(const TypeID& original)
	: typeInfo_(original.typeInfo_)
	, module_(original.module_)
#if !YASLI_NO_RTTI
	, name_(original.name_)
#endif
	{
	}

#if !YASLI_NO_RTTI
	explicit TypeID(const type_info& typeInfo)
	: typeInfo_(&typeInfo)
	{
	}
#endif

	operator bool() const{
		return *this != TypeID();
	}

	template<class T>
	static TypeID get();
	std::size_t sizeOf() const;
	const char* name() const;

	bool operator==(const TypeID& rhs) const;
	bool operator!=(const TypeID& rhs) const;
	bool operator<(const TypeID& rhs) const;

#if !YASLI_NO_RTTI
	typedef const type_info TypeInfo;
#endif
	const TypeInfo* typeInfo() const{ return typeInfo_; }
private:
	TypeInfo* typeInfo_;
#if YASLI_NO_RTTI 
	friend class bTypeInfo;
#else
	string name_;
#endif
	void* module_;
	friend class TypeDescription;
#if YASLI_NO_RTTI
	friend struct TypeInfo;
#endif
};

#if YASLI_NO_RTTI
struct TypeInfo
{
	TypeID id;
	size_t size;
	char name[256];

    // Remove class/struct prefixes and whitespaces
    static void cleanTypeName(char*& d, const char* dend, const char*& s, const char* send);
	static void extractTypeName(char *name, int nameLen, const char* funcName);

	TypeInfo(size_t size, const char* templatedFunctionName)
	: size(size)
	{
		extractTypeName(name, sizeof(name), templatedFunctionName);
		id.typeInfo_ = this;
		static int moduleSpecificSymbol;
		id.module_ = &moduleSpecificSymbol;
	}

	bool operator==(const TypeInfo& rhs) const{
		return size == rhs.size && strcmp(name, rhs.name) == 0;
	}

	bool operator<(const TypeInfo& rhs) const{
		if (size == rhs.size)
			return strcmp(name, rhs.name) < 0;
		else
			return size < rhs.size;
	}
};
#endif

template<class T>
TypeID TypeID::get()
{
#if YASLI_NO_RTTI
# ifdef _MSC_VER
	static TypeInfo typeInfo(sizeof(T), __FUNCSIG__);
# else
	static TypeInfo typeInfo(sizeof(T), __PRETTY_FUNCTION__);
# endif
	return typeInfo.id;
#else
	static TypeID result(typeid(T));
	return result;
#endif
}

template<class T>
T* createDerivedClass(TypeID typeID);

YASLI_INLINE void makePrettyTypeName(char*& d, const char* dend, const char*& s, const char* send);
YASLI_INLINE yasli::string makePrettyTypeName(const char* typeName);

}

#if YASLI_INLINE_IMPLEMENTATION
#include "TypeID.cpp"
#endif
