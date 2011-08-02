/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/MemoryWriter.h"
#include "yasli/TypesFactory.h"

namespace yasli{

template<class T>
class SharedPtrSerializationImpl : public PointerSerializationInterface
{
public:
	SharedPtrSerializationImpl(SharedPtr<T>& ptr)
	: ptr_(ptr)
	{}

	TypeID type() const{
		if(ptr_)
			return TypeID::get(ptr_.get());
		else
			return TypeID();
	}
	void create(TypeID type) const{
		ASSERT(!ptr_ || ptr_->refCount() == 1);
		if(type)
			ptr_.set(ClassFactory<T>::the().create(type));
		else
			ptr_.set((T*)0);
	}
	TypeID baseType() const{ return TypeID::get<T>(); }
	virtual Serializer serializer() const{
		return Serializer(*ptr_);
	}
	void* get() const{
		return reinterpret_cast<void*>(ptr_.get());
	}
	void extractInPlacePointers(Archive& ar) const
	{
		ar.inPlacePointer(reinterpret_cast<void**>(&ptr_), 0);
	}
	ClassFactoryBase* factory() const{ return &ClassFactory<T>::the(); }
protected:
	mutable SharedPtr<T>& ptr_;
};

template<class T>
class PolyPtrSerializationImpl : public PointerSerializationInterface
{
public:
	PolyPtrSerializationImpl(PolyPtr<T>& ptr)
	: ptr_(ptr)
	{}

	TypeID type() const{
		if(ptr_)
			return TypeID::get(ptr_.get());
		else
			return TypeID();
	}
	void create(TypeID type) const{
		// ASSERT(!ptr_ || ptr_->refCount() == 1); not necessary to be true
		if(type)
			ptr_.set(ClassFactory<T>::the().create(type));
		else
			ptr_.set((T*)0);
	}
	TypeID baseType() const{ return TypeID::get<T>(); }
	virtual Serializer serializer() const{
		return Serializer(*ptr_);
	}
	void* get() const{
		return reinterpret_cast<void*>(ptr_.get());
	}
	void extractInPlacePointers(Archive& ar) const
	{
		ar.inPlacePointer(reinterpret_cast<void**>(&ptr_), 0);
	}
	ClassFactoryBase* factory() const{ return &ClassFactory<T>::the(); }
protected:
	mutable PolyPtr<T>& ptr_;
};


template<class T>
void SharedPtr<T>::serialize(Archive& ar)
{
    static TypeID baseTypeID = TypeID::get<T>();

    TypeID oldTypeID;
    if(*this)
        oldTypeID = TypeID::get(&**this);

    if(ar.isOutput()){
        if(*this){
            if(ar(oldTypeID, "", "<")){
                ASSERT(*this);
                ar(**this, "", "<");
            }
            else
                ar.warning("Unable to write typeID!");
        }
    }
    else{
        TypeID typeID;
        if(!ar(typeID, "", "<")){
            if(*this){
                ASSERT((*this)->refCount() == 1);
                this->set((T*)0);
            }
            return;
        }

        if(oldTypeID && (!typeID || (typeID != oldTypeID))){
            ASSERT((*this)->refCount() == 1);
            this->set((T*)0);
        }

        if(typeID){
            if(!this->get()){
                T* ptr = ClassFactory<T>::the().create(typeID);
                ASSERT(ptr);
                this->set(ptr);
                ASSERT(this->get());
            }
            ASSERT(*this);
            ar(**this, "", "<");
        }
    }
}

}

template<class T>
bool serialize(yasli::Archive& ar, yasli::SharedPtr<T>& ptr, const char* name, const char* label)
{
	return ar(static_cast<const yasli::PointerSerializationInterface&>(yasli::SharedPtrSerializationImpl<T>(ptr)), name, label);
}

template<class T>
bool serialize(yasli::Archive& ar, yasli::PolyPtr<T>& ptr, const char* name, const char* label)
{
	return ar(static_cast<const yasli::PointerSerializationInterface&>(yasli::PolyPtrSerializationImpl<T>(ptr)), name, label);
}
