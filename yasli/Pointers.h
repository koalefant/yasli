/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "yasli/Assert.h"

namespace yasli{

class RefCounter{
public:
    RefCounter()
    : refCounter_(0)
    {}
    ~RefCounter() {};

    int refCount() const{ return refCounter_; }

    void acquire(){ ++refCounter_; }
    int release(){ return --refCounter_; }
private:
    int refCounter_;
};

class PolyRefCounter : public RefCounter{
public:
    virtual ~PolyRefCounter() {}
};

class PolyPtrBase{
public:
    PolyPtrBase()
    : ptr_(0)
    {
    }
    void release(){
        if(ptr_){
            if(!ptr_->release())
                delete ptr_;
            ptr_ = 0;
        }
    }
    void reset(PolyRefCounter* const ptr = 0){
        if(ptr_ != ptr){
            release();
            ptr_ = ptr;
            if(ptr_)
                ptr_->acquire();
        }
    }
protected:
    PolyRefCounter* ptr_;
};

template<class T>
class PolyPtr : public PolyPtrBase{
public:
    PolyPtr()
    : PolyPtrBase()
    {
    }

    PolyPtr(PolyRefCounter* ptr)
    {
        reset(ptr);
    }

	template<class U>
    PolyPtr(U* ptr)
    {
		// TODO: replace with static_assert
		YASLI_ASSERT("PolyRefCounter must be a first base when used with multiple inheritance." && 
			   static_cast<PolyRefCounter*>(ptr) == reinterpret_cast<PolyRefCounter*>(ptr));
        reset(static_cast<PolyRefCounter*>(ptr));
    }

    PolyPtr(const PolyPtr& ptr)
    : PolyPtrBase()
    {
        reset(ptr.ptr_);
    }
    ~PolyPtr(){
        release();
    }
    operator T*() const{ return get(); }
    template<class U>
    operator PolyPtr<U>() const{ return PolyPtr<U>(get()); }
    operator bool() const{ return ptr_ != 0; }

	PolyPtr& operator=(const PolyPtr& ptr){
        reset(ptr.ptr_);
        return *this;
    }
    T* get() const{ return reinterpret_cast<T*>(ptr_); }
    T& operator*() const{
        return *get();
    }
    T* operator->() const{ return get(); }
};

class Archive;
template<class T>
class SharedPtr{
public:
    SharedPtr()
    : ptr_(0){}
    SharedPtr(T* const ptr)
    : ptr_(0)
    {
        reset(ptr);
    }
    SharedPtr(const SharedPtr& ptr)
    : ptr_(0)
    {
        reset(ptr.ptr_);
    }
    ~SharedPtr(){
        release();
    }
    operator T*() const{ return get(); }
    template<class U>
    operator SharedPtr<U>() const{ return SharedPtr<U>(get()); }
    SharedPtr& operator=(const SharedPtr& ptr){
        reset(ptr.ptr_);
        return *this;
    }
    T* get(){ return ptr_; }
    T* get() const{ return ptr_; }
    T& operator*(){
        return *get();
    }
    T* operator->() const{ return get(); }
    void release(){
			if(ptr_){
				if(!ptr_->release())
					delete ptr_;
				ptr_ = 0;
			}
    }
	template<class _T>
    void reset(_T* const ptr){
        if(ptr_ != ptr){
            release();
            ptr_ = ptr;
            if(ptr_)
                ptr_->acquire();
        }
    }

protected:
    T* ptr_;
};


template<class T>
class AutoPtr{
public:
    AutoPtr()
    : ptr_(0)
    {
    }
    AutoPtr(T* ptr)
    : ptr_(0)
    {
        set(ptr);
    }
    ~AutoPtr(){
        release();
    }
    AutoPtr& operator=(T* ptr){
        set(ptr);
        return *this;
    }
    void set(T* ptr){
        if(ptr_ && ptr_ != ptr)
            release();
        ptr_ = ptr;
        
    }
    T* get() const{ return ptr_; }
    operator T*() const{ return get(); }
    void detach(){
        ptr_ = 0;
    }
    void release(){
        delete ptr_;
        ptr_ = 0;
    }
    T& operator*() const{ return *get(); }
    T* operator->() const{ return get(); }
private:
    T* ptr_;
};

class Archive;


template<class Ptr>
struct AsObjectWrapper
{
	Ptr& ptr_;

	AsObjectWrapper(Ptr& ptr)
	: ptr_(ptr)
	{
	}
};

}

template<class T>
bool serialize(yasli::Archive& ar, yasli::SharedPtr<T>& ptr, const char* name, const char* label);

template<class T>
bool serialize(yasli::Archive& ar, yasli::PolyPtr<T>& ptr, const char* name, const char* label);


template<class T>
bool serialize(yasli::Archive& ar, yasli::AsObjectWrapper<yasli::SharedPtr<T> >& ptr, const char* name, const char* label);