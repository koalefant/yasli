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
	enum { SIGNATURE = 987192387 };
	virtual int signature() const { return SIGNATURE; }
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
    void set(PolyRefCounter* const ptr){
        if(ptr_ != ptr){
			ASSERT(!ptr || ptr->signature() == PolyRefCounter::SIGNATURE); // You should derive Your class from PolyRefCounter (not RefCounter) in the first place (to dynamic_cast == reinterpret_cast)
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
    PolyPtr(T* const ptr)
    : PolyPtrBase()
    {
        set(reinterpret_cast<PolyRefCounter*>(ptr));
    }
    PolyPtr(const PolyPtr& ptr)
    : PolyPtrBase()
    {
        set(reinterpret_cast<PolyRefCounter*>(ptr.ptr_));
    }
    ~PolyPtr(){
        release();
    }
    operator T*() const{ return get(); }
    template<class U>
    operator PolyPtr<U>() const{ return PolyPtr<U>(get()); }
    operator bool() const{ return ptr_ != 0; }
    PolyPtr& operator=(const PolyPtr& ptr){
        set(ptr.ptr_);
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
        set(ptr);
    }
    SharedPtr(const SharedPtr& ptr)
    : ptr_(0)
    {
        set(ptr.ptr_);
    }
    ~SharedPtr(){
        release();
    }
    operator T*() const{ return get(); }
    template<class U>
    operator SharedPtr<U>() const{ return SharedPtr<U>(get()); }
    SharedPtr& operator=(const SharedPtr& ptr){
        set(ptr.ptr_);
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
    void set(_T* const ptr){
        if(ptr_ != ptr){
            release();
            ptr_ = ptr;
            if(ptr_)
                ptr_->acquire();
        }
    }

    void serialize(Archive& ar);
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

}

template<class T>
bool serialize(yasli::Archive& ar, yasli::SharedPtr<T>& ptr, const char* name, const char* label);

template<class T>
bool serialize(yasli::Archive& ar, yasli::PolyPtr<T>& ptr, const char* name, const char* label);
