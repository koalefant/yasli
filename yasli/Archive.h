/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#pragma warning (disable : 4100) 

#include <map>

#include "yasli/Helpers.h"
#include "yasli/Serializer.h"
#include "yasli/TypeID.h"

namespace yasli{ class Archive; }

template<class T>
bool serialize(yasli::Archive& ar, T& object, const char* name, const char* label);

namespace yasli{

class Object;
class EnumDescription;
template <class Enum>
EnumDescription& getEnumDescription();
bool serializeEnum(const EnumDescription& desc, Archive& ar, int& value, const char* name, const char* label);

class Archive{
public:
	template<class T>
	class Context{
	public:
		Context(Archive& ar, T* context) : ar_(ar) { previousContext_ = ar_.setContext(context); }
		~Context() { ar_.setContext( previousContext_ ); }

		T* previous() const{ return previousContext_; }
	private:
		Archive &ar_;
		T* previousContext_;
	};

	enum ArchiveCaps{
		INPUT = 1 << 0,
		OUTPUT = 1 << 1,
		TEXT = 1 << 2,
		BINARY = 1 << 3,
		EDIT = 1 << 4,
		INPLACE = 1 << 5,
		CUSTOM1 = 1 << 6,
		CUSTOM2 = 1 << 7,
		CUSTOM3 = 1 << 8
	};

	Archive(int caps)
	: caps_(caps)
	, filter_(0)
	{
	}
	virtual ~Archive() {}

	bool isInput() const{ return caps_ & INPUT ? true : false; }
	bool isOutput() const{ return caps_ & OUTPUT ? true : false; }
	bool isEdit() const{ return caps_ & EDIT ? true : false; }
	bool isInPlace() const{ return caps_ & INPLACE ? true : false; }
	bool caps(int caps) const { return (caps_ & caps) == caps; }

	void setFilter(int filter){
		filter_ = filter;
	}
	int getFilter() const{ return filter_; }
	bool filter(int flags) const{
		YASLI_ASSERT(flags != 0 && "flags is supposed to be a bit mask");
		YASLI_ASSERT(filter_ && "Filter is not set!");
		return (filter_ & flags) != 0;
	}

	virtual void warning(const char* message) {}
	virtual void inPlacePointer(void** pointer, size_t offset) { YASLI_ASSERT(0 && "Not implemented"); }
	virtual bool operator()(bool& value, const char* name = "", const char* label = 0)           { notImplemented(); return false; }
	virtual bool operator()(unsigned char& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(signed char& value, const char* name = "", const char* label = 0)   { notImplemented(); return false; }
	virtual bool operator()(char& value, const char* name = "", const char* label = 0)          { notImplemented(); return false; }
	virtual bool operator()(short& value, const char* name = "", const char* label = 0)   { notImplemented(); return false; }
	virtual bool operator()(unsigned short& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(int& value, const char* name = "", const char* label = 0)            { notImplemented(); return false; }
	virtual bool operator()(unsigned int& value, const char* name = "", const char* label = 0)   { notImplemented(); return false; }
	virtual bool operator()(long long& value, const char* name = "", const char* label = 0)        { notImplemented(); return false; }
	virtual bool operator()(unsigned long long& value, const char* name = "", const char* label = 0)        { notImplemented(); return false; }
	virtual bool operator()(float& value, const char* name = "", const char* label = 0)          { notImplemented(); return false; }
	virtual bool operator()(double& value, const char* name = "", const char* label = 0)         { notImplemented(); return false; }

	virtual bool operator()(StringInterface& value, const char* name = "", const char* label = 0)    { notImplemented(); return false; }
	virtual bool operator()(WStringInterface& value, const char* name = "", const char* label = 0)    { notImplemented(); return false; }
	virtual bool operator()(const Serializer& ser, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(ContainerInterface& ser, const char* name = "", const char* label = 0) { return false; }
	virtual bool operator()(PointerInterface& ptr, const char* name = "", const char* label = 0);
	virtual bool operator()(Object& obj, const char* name = "", const char* label = 0) { return false; }

  // there is no point to support long double since it is represented as double on MSVC
	bool operator()(long double& value, const char* name = "", const char* label = 0)         { notImplemented(); return false; }
	// fall back to int implementation for long
	bool operator()(long& value, const char* name = "", const char* label = 0) { return operator()(*reinterpret_cast<int*>(&value), name, label); }
	bool operator()(unsigned long& value, const char* name = "", const char* label = 0) { return operator()(*reinterpret_cast<unsigned int*>(&value), name, label); }

	// block call are osbolete, please do not use
	virtual bool openBlock(const char* name, const char* label) { return true; }
	virtual void closeBlock() {}

	// templated switch
	template<class T>
	bool operator()(const T& value, const char* name = "", const char* label = 0);

	// for compatibility
	template<class T>
	bool serialize(const T& value, const char* name, const char* label)
	{
		return operator()(const_cast<T&>(value), name, label);
	}

	template<class T>
	T* context()
	{
		ContextMap::iterator it = contextMap_.find(TypeID::get<T>());
		if(it == contextMap_.end())
			return 0;
		return reinterpret_cast<T*>(it->second);
	}
	template<class T>
	T* setContext(T* c)
	{
		void*& ptr = contextMap_[TypeID::get<T>()];
		T* result = reinterpret_cast<T*>(ptr);
		ptr = reinterpret_cast<void*>(c);
		return result;
	}
	typedef std::map<TypeID, void*> ContextMap;
	void setContextMap(const ContextMap& contextMap){ contextMap_ = contextMap; }
	const ContextMap& contextMap() const{ return contextMap_; }
protected:
	ContextMap contextMap_;
	int caps_;

private:
	void notImplemented() { YASLI_ASSERT(0 && "Not implemented!"); }

	int filter_;
};

namespace Helpers{

template<class T>
struct SerializeStruct{
	static bool invoke(Archive& ar, T& value, const char* name, const char* label){
		return ar(Serializer(value), name, label);
	};
};

template<class Enum>
struct SerializeEnum{
	static bool invoke(Archive& ar, Enum& value, const char* name, const char* label){
		const EnumDescription& enumDescription = getEnumDescription<Enum>();
		return serializeEnum(enumDescription, ar, reinterpret_cast<int&>(value), name, label);
	};
};

template<class T>
struct SerializeArray{};

template<class T, int Size>
struct SerializeArray<T[Size]>{
	static bool invoke(Archive& ar, T value[Size], const char* name, const char* label){
		ContainerArray<T> ser(value, Size);
		return ar(static_cast<ContainerInterface&>(ser), name, label);
	}
};

}

template<class T>
bool Archive::operator()(const T& value, const char* name, const char* label){
    return ::serialize(*this, const_cast<T&>(value), name, label);
}

inline bool Archive::operator()(PointerInterface& ptr, const char* name, const char* label)
{
	return (*this)(Serializer(const_cast<PointerInterface&>(ptr)), name, label);
}

}

template<class T, int Size>
bool serialize(yasli::Archive& ar, T object[Size], const char* name, const char* label)
{
	YASLI_ASSERT(0);
	return false;
}

template<class T>
bool serialize(yasli::Archive& ar, const T& object, const char* name, const char* label)
{
	T::unable_to_serialize_CONST_object();
	YASLI_ASSERT(0);
	return false;
}

template<class T>
bool serialize(yasli::Archive& ar, T& object, const char* name, const char* label)
{
	using namespace yasli::Helpers;

	return
		Select< IsClass<T>,
				Identity< SerializeStruct<T> >,
				Select< IsArray<T>,
					Identity< SerializeArray<T> >,
					Identity< SerializeEnum<T> >
				>
		>::type::invoke(ar, object, name, label);
}

#include "yasli/SerializerImpl.h"

// vim: ts=4 sw=4:
