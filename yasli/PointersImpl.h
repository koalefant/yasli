#pragma once

#include "utils/MemoryWriter.h"
#include "utils/SafeCast.h"
#include "yasli/TypesFactory.h"

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
		ptr_.set(ClassFactory<T>::the().create(type));
	}
	TypeID baseType() const{ return TypeID::get<T>(); }
	virtual Serializer serializer() const{
		ASSERT(ptr_);
		return Serializer(*ptr_);
	}
	void* get() const{
		return reinterpret_cast<void*>(ptr_.get());
	}

protected:
	mutable SharedPtr<T>& ptr_;
};


template<class T>
class SerializeableSharedPtr : public SharedPtr<T>{
public:
    typedef SharedPtr<T> Super;
    void serialize(Archive& ar){
        static TypeID baseTypeID = TypeID::get<T>();

        TypeID oldTypeID;
        if(*this)
            oldTypeID = TypeID::get(&**this);

        if(ar.isOutput()){
            if(*this){
                if(ar(oldTypeID, "")){
                    ASSERT(*this);
                    ar(**this, "");
                }
                else
                    ar.warning("Unable to write typeID!");
            }
        }
        else{
            TypeID typeID;
            if(!ar(typeID, "")){
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
                ar(**this, "");
            }
        }
    }
};

template<class T>
bool serialize(Archive& ar, SharedPtr<T>& ptr, const char* name)
{
	return ar(SharedPtrSerializationImpl<T>(ptr), name);
}
