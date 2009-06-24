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

#ifndef WIN32
typedef long long __int64;
#endif


template<class T>
bool serialize(Archive& ar, T& object, const char* name, const char* label);

class YASLI_API Archive : public RefCounter{
public:
    Archive(bool isInput, bool isEdit = false)
    : isInput_(isInput)
    , isEdit_(isEdit)
    , inPlace_(false)
	, filter_(0)
    {
    }
    virtual ~Archive() {}

    bool isPermanent() const{ return true; }
    bool isInput() const{ return isInput_; }
    bool isOutput() const{ return !isInput_; }
    bool isEdit() const{ return isEdit_; }
    bool inPlace() const{ return inPlace_; }

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
    virtual bool operator()(bool& value, const char* name = "", const char* label = 0)          { return false; }
    virtual bool operator()(std::string& value, const char* name = "", const char* label = 0)   { return false; }
    virtual bool operator()(float& value, const char* name = "", const char* label = 0)         { return false; }
    virtual bool operator()(double& value, const char* name = "", const char* label = 0)         { return false; }
    virtual bool operator()(int& value, const char* name = "", const char* label = 0)           { return false; }
    virtual bool operator()(__int64& value, const char* name = "", const char* label = 0)       { return false; }

	bool operator()(long& value, const char* name = "", const char* label = 0) { return operator()(*reinterpret_cast<int*>(&value), name, label); }
	bool operator()(unsigned long& value, const char* name = "", const char* label = 0) { return operator()(*reinterpret_cast<unsigned int*>(&value), name, label); }

	virtual bool operator()(unsigned short& value, const char* name = "", const char* label = 0) { return false; }
    virtual bool operator()(signed short& value, const char* name = "", const char* label = 0)   { return false; }
    
    virtual bool operator()(unsigned char& value, const char* name = "", const char* label = 0) { return false; }
    virtual bool operator()(signed char& value, const char* name = "", const char* label = 0)   { return false; }
    virtual bool operator()(char& value, const char* name = "", const char* label = 0)          { return false; }

    virtual bool operator()(const Serializer& ser, const char* name = "", const char* label = 0) { return false; }
    virtual bool operator()(ContainerSerializationInterface& ser, const char* name = "", const char* label = 0);
	virtual bool operator()(const PointerSerializationInterface& ptr, const char* name = "", const char* label = 0);

    // templated switch
    template<class T>
    bool operator()(const T& value, const char* name = "", const char* label = 0);

	template<class T>
	bool serialize(const T& value, const char* name, const char* label)
	{
		return operator()(const_cast<T&>(value), name, label);
	}
private:
    bool isInput_;
    bool isEdit_;
	bool inPlace_;
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

    template<class Enum>
    struct SerializeEnum{
        static bool invoke(Archive& ar, Enum& value, const char* name, const char* label){
            const EnumDescription& enumDescription = getEnumDescription<Enum>();
            ASSERT(enumDescription.registered());
            return enumDescription.serialize(ar, reinterpret_cast<int&>(value), name, label);
        };
    };

    template<class T>
    struct SerializeArray{};

    template<class T, int Size>
    struct SerializeArray<T[Size]>{
        static bool invoke(Archive& ar, T value[Size], const char* name, const char* label){
            return ar(static_cast<const ContainerSerializationInterface&>(ContainerSerializationArrayImpl<T>(value, Size)), name, label);
        }
    };
}


template<class T, int Size>
bool serialize(Archive& ar, T object[Size], const char* name, const char* label)
{
    ASSERT(0);
    return false;
}

template<class T>
bool serialize(Archive& ar, const T& object, const char* name, const char* label)
{
    T::unable_to_serialize_CONST_object();
    ASSERT(0);
    return false;
}

template<class T>
bool serialize(Archive& ar, T& object, const char* name, const char* label)
{
    using namespace Helpers;

    return
        Select< IsClass<T>,
                Identity< SerializeStruct<T> >,
				Select< IsArray<T>,
					Identity< SerializeArray<T> >,
					Identity< SerializeEnum<T> >
				>
        >::type::invoke(ar, object, name, label);
}

template<class T>
bool Archive::operator()(const T& value, const char* name, const char* label){
    return ::serialize(*this, const_cast<T&>(value), name, label);
}
