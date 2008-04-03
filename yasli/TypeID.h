#pragma once

#include <typeinfo>
#include "yasli/Errors.h"
#include "yasli/API.h"

#ifndef _MSC_VER
using std::type_info;
#endif

class Archive;
class TypeID;

bool YASLI_API serialize(Archive& ar, TypeID& typeID, const char* name);

class YASLI_API TypeID{
    friend class TypesFactory;
public:
    static TypeID ZERO;
    static TypeID UNKNOWN;

    TypeID() : typeInfo_(0) {}

    TypeID(const TypeID& original)
    : typeInfo_(original.typeInfo_)
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
    bool registered() const;

    bool operator==(const TypeID& rhs) const{
        return typeInfo_ == rhs.typeInfo_;
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
};

template<class T>
T* createDerivedClass(TypeID typeID);
