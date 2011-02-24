#pragma once

#include <string>
#include <iostream>

#include "yasli/Pointers.h"
#include "yasli/API.h"
#include "yasli/TypeID.h"
#include "yasli/Helpers.h"
#include "yasli/Serializer.h"
#include "yasli/StringList.h"
#include "yasli/EnumDescription.h"

#pragma warning (disable: 4100)

namespace yasli{ class Archive; }

template<class T>
bool serialize(yasli::Archive& ar, T& object, const char* name, const char* label);

namespace yasli{

class YASLI_API Archive : public PolyRefCounter{
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
		EDIT = 1 << 1,
		INPLACE = 1 << 2,
	};

	Archive(bool isInput, bool isEdit = false)
	: isInput_(isInput)
	, isEdit_(isEdit)
	, isInPlace_(false)
	, filter_(0)
	{
	}

	Archive(int caps)
	: isInput_((caps & INPUT) != 0)
	, isEdit_((caps & EDIT) != 0)
	, isInPlace_((caps & INPLACE) != 0)
	, filter_(0)
	{

	}

	virtual ~Archive() {}

	bool isPermanent() const{ return true; }
	bool isInput() const{ return isInput_; }
	bool isOutput() const{ return !isInput_; }
	bool isEdit() const{ return isEdit_; }
	bool isInPlace() const{ return isInPlace_; }
	virtual void inPlacePointer(void** pointer, size_t offset) { ASSERT(0 && "Not implemented"); }

	virtual void warning(const char* message);
	virtual void close() {}
	virtual void clear() {};

	void setFilter(int filter){
		filter_ = filter;
	}
	int getFilter() const{ return filter_; }
	bool filter(int flags) const{
		ASSERT(filter_ && "Filter is not set!");
		return (filter_ & flags) != 0;
	}

	virtual const char* pull(){ return 0; }

	// basic types
	virtual bool operator()(bool& value, const char* name = "", const char* label = 0)           { notImplemented(); return false; }
	virtual bool operator()(std::string& value, const char* name = "", const char* label = 0)    { notImplemented(); return false; }
	virtual bool operator()(std::wstring& value, const char* name = "", const char* label = 0);
	virtual bool operator()(float& value, const char* name = "", const char* label = 0)          { notImplemented(); return false; }
	virtual bool operator()(double& value, const char* name = "", const char* label = 0)         { notImplemented(); return false; }

	virtual bool operator()(int& value, const char* name = "", const char* label = 0)            { notImplemented(); return false; }
	virtual bool operator()(unsigned int& value, const char* name = "", const char* label = 0)   { notImplemented(); return false; }

	bool operator()(long& value, const char* name = "", const char* label = 0) { return operator()(*reinterpret_cast<int*>(&value), name, label); }
	bool operator()(unsigned long& value, const char* name = "", const char* label = 0) { return operator()(*reinterpret_cast<unsigned int*>(&value), name, label); }

	virtual bool operator()(long long& value, const char* name = "", const char* label = 0)        { notImplemented(); return false; }

	virtual bool operator()(unsigned short& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(signed short& value, const char* name = "", const char* label = 0)   { notImplemented(); return false; }

	virtual bool operator()(unsigned char& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(signed char& value, const char* name = "", const char* label = 0)   { notImplemented(); return false; }
	virtual bool operator()(char& value, const char* name = "", const char* label = 0)          { notImplemented(); return false; }

	virtual bool operator()(const Serializer& ser, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(ContainerSerializationInterface& ser, const char* name = "", const char* label = 0);
	virtual bool operator()(const PointerSerializationInterface& ptr, const char* name = "", const char* label = 0);

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
		ContextMap::iterator it = contextMap_.find(&typeid(T));
		if(it == contextMap_.end())
			return 0;
		return reinterpret_cast<T*>(it->second);
	}
	template<class T>
	T* setContext(T* c)
	{
		void*& ptr = contextMap_[&typeid(T)];
		T* result = reinterpret_cast<T*>(ptr);
		ptr = reinterpret_cast<void*>(c);
		return result;
	}
	typedef std::map<const type_info*, void*> ContextMap;
	void setContextMap(const ContextMap& contextMap){ contextMap_ = contextMap; }
	const ContextMap& contextMap() const{ return contextMap_; }
protected:
	ContextMap contextMap_;

private:
	void notImplemented();

	bool isInput_;
	bool isEdit_;
	bool isInPlace_;
	int filter_;
};

namespace Helpers{
template<class T>
struct SerializeMethodForm{
	struct YesType{ char dummy[100]; };
	struct NoType{ char dummy[1]; };

	static NoType  testFunc(void (T::*f)(Archive& ar));
	static YesType testFunc(bool (T::*f)(Archive& ar, const char*));

	enum { value = (sizeof(testFunc(&T::serialize)) == sizeof(YesType)) };
};

template<class T>
struct SerializeStruct{
	static bool invoke(Archive& ar, T& value, const char* name, const char* label){
		return ar(Serializer(value), name, label);
	};
};

template<class T>
struct SerializeStructWithName{
	static bool invoke(Archive& ar, T& value, const char* name, const char* label){
		return value.serialize(ar, name, label);
	};
};

template<class T>
struct SerializeStructFast{
	template<class TArchive>
	static bool invoke(TArchive& ar, T& value, const char* name, const char* label){
		return ar.serializeStruct(value, name, label);
	};
};

template<class Enum>
struct SerializeEnum{
	static bool invoke(Archive& ar, Enum& value, const char* name, const char* label){
		const EnumDescription& enumDescription = getEnumDescription<Enum>();
		ASSERT(enumDescription.registered());
		return enumDescription.serialize(ar, reinterpret_cast<int&>(value), name, label);
	};
};

template<class Enum>
struct SerializeEnumFast{
	template<class TArchive>
	static bool invoke(TArchive& ar, Enum& value, const char* name, const char* label){
		return ar((int&)value, name, label); 
	};
};

template<class T>
struct SerializeArray{};

template<class T, int Size>
struct SerializeArray<T[Size]>{
	static bool invoke(Archive& ar, T value[Size], const char* name, const char* label){
		ContainerSerializationArrayImpl<T> ser(value, Size);
		return ar(static_cast<ContainerSerializationInterface&>(ser), name, label);
	}
};
}

template<class T>
bool Archive::operator()(const T& value, const char* name, const char* label){
    return ::serialize(*this, const_cast<T&>(value), name, label);
}

}

template<class T, int Size>
bool serialize(yasli::Archive& ar, T object[Size], const char* name, const char* label)
{
	ASSERT(0);
	return false;
}

template<class T>
bool serialize(yasli::Archive& ar, const T& object, const char* name, const char* label)
{
	T::unable_to_serialize_CONST_object();
	ASSERT(0);
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
