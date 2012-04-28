/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "Pointers.h"
#include "yasli/ClassFactoryBase.h"
#include "ClassFactory.h"

namespace yasli{

template<class T>
class SharedPtrSerializer : public PointerInterface
{
public:
	SharedPtrSerializer(SharedPtr<T>& ptr)
	: ptr_(ptr)
	{}

	TypeID type() const{
		if(ptr_)
			return TypeID::get(ptr_.get());
		else
			return TypeID();
	}
	void create(TypeID type) const{
		YASLI_ASSERT(!ptr_ || ptr_->refCount() == 1);
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
class PolyPtrSerializer : public PointerInterface
{
public:
	PolyPtrSerializer(PolyPtr<T>& ptr)
	: ptr_(ptr)
	{}

	TypeID type() const{
		if(ptr_)
			return TypeID::get(ptr_.get());
		else
			return TypeID();
	}
	void create(TypeID type) const{
		// YASLI_ASSERT(!ptr_ || ptr_->refCount() == 1); not necessary to be true
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


}

template<class T>
bool serialize(yasli::Archive& ar, yasli::SharedPtr<T>& ptr, const char* name, const char* label)
{
	return ar(static_cast<yasli::PointerInterface&>(yasli::SharedPtrSerializer<T>(ptr)), name, label);
}

template<class T>
bool serialize(yasli::Archive& ar, yasli::PolyPtr<T>& ptr, const char* name, const char* label)
{
	return ar(static_cast<yasli::PointerInterface&>(yasli::PolyPtrSerializer<T>(ptr)), name, label);
}
// vim:sw=4 ts=4:
