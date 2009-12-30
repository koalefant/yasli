#pragma once

#include <typeinfo>
#include "yasli/Assert.h"
#include "yasli/API.h"
#include <string>

#ifndef _MSC_VER
using std::type_info;
#endif

namespace yasli{

class Archive;
class TypeID;

class YASLI_API TypeID{
    friend class TypesFactory;
public:
    static TypeID ZERO;
    static TypeID UNKNOWN;

    TypeID() : typeInfo_(0) {}

    TypeID(const TypeID& original)
    : typeInfo_(original.typeInfo_)
	, name_(original.name_)
    {
    }

    explicit TypeID(const type_info& typeInfo)
    : typeInfo_(&typeInfo)
    {
    }

    explicit TypeID(const char* name);

    operator bool() const{
        return *this != ZERO;
    }

    template<class T>
    static TypeID get(){
        static TypeID result(typeid(T));
        return result;
    }
    template<class T>
    static TypeID get(T* t){
        return TypeID(typeid(*t));
    }
    std::size_t sizeOf() const;
    const char* name() const;
    const char* label() const;
    bool registered() const;

    bool operator==(const TypeID& rhs) const{
		if(typeInfo_ && rhs.typeInfo_)
			return typeInfo_ == rhs.typeInfo_;
		else
		{
			const char* name1 = name();
			const char* name2 = rhs.name();
			return strcmp(name1, name2) == 0;
		}
    }
    bool operator!=(const TypeID& rhs) const{
        return typeInfo_ != rhs.typeInfo_;
    }
    bool operator<(const TypeID& rhs) const{
        if(typeInfo_ && rhs.typeInfo_)
            return typeInfo_->before(*rhs.typeInfo_) > 0;
        else if(!typeInfo_)
            return rhs.typeInfo_!= 0;
        else if(!rhs.typeInfo_)
            return false;
        return false;
    }

    void* create() const;
    const type_info* typeInfo() const{ return typeInfo_; }
private:
    const type_info* typeInfo_;
	std::string name_;
};

template<class T>
T* createDerivedClass(TypeID typeID);

}

bool YASLI_API serialize(yasli::Archive& ar, yasli::TypeID& typeID, const char* name, const char* label);
