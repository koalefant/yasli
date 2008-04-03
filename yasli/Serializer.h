#pragma once

#include "yasli/Assert.h"
#include "yasli/API.h"
#include "yasli/TypeID.h"

class Archive;

typedef bool(*SerializeStructFunc)(void*, Archive&);

typedef bool(*SerializeContainerFunc)(void*, Archive&, std::size_t index);
typedef std::size_t(*ContainerResizeFunc)(void*, std::size_t size);
typedef std::size_t(*ContainerSizeFunc)(void*);

class YASLI_API Serializer{/*{{{*/
    friend class Archive;
public:
    Serializer()
    : object_(0)
	, size_(0)
    , serializeFunc_(0)
    {
    }

	Serializer(TypeID type, void* object, std::size_t size, SerializeStructFunc serialize)
    : type_(type)
    , object_(object)
	, size_(size)
    , serializeFunc_(serialize)
    {
        ASSERT(object);
    }

    template<class T>
    explicit Serializer(T& object){
        type_=  TypeID::get<T>();
        object_ = (void*)(&object);
		size_ = sizeof(T);
        ASSERT(object_);
        serializeFunc_ = &Serializer::serializeRaw<T>;
    }

    template<class T>
    explicit Serializer(T& object, TypeID type){
        type_ =  type;
        object_ = (void*)(&object);
		size_ = sizeof(T);
        ASSERT(object_);
        serializeFunc_ = &Serializer::serializeRaw<T>;
    }

    bool operator()(Archive& ar, const char* name) const;
    bool operator()(Archive& ar) const;
    operator bool() const{ return object_ && serializeFunc_; }
    void* pointer() const{ return object_; }
    TypeID type() const{ return type_; }
	std::size_t size() const{ return size_; }

    template<class T>
    static bool serializeRaw(void* rawPointer, Archive& ar){
        ASSERT(rawPointer);
        ((T*)(rawPointer))->serialize(ar);
        //::serialize(ar, *((T*)(rawPointer)), "name");
        return true;
    }
private:

    TypeID type_;
    void* object_;
	std::size_t size_;
    SerializeStructFunc serializeFunc_;
};/*}}}*/

class ContainerSerializationInterface{
public:
    virtual std::size_t size() const = 0;
    virtual std::size_t resize(std::size_t size) const = 0;

    virtual void* pointer() const = 0;
    virtual TypeID type() const = 0;

    virtual bool operator()(Archive& ar, std::size_t index) const = 0;
    virtual operator bool() const = 0;
    virtual void serializeNewElement(Archive& ar, const char* name = "") const = 0;
};

template<class Container, class Element>
class ContainerSerializationSTLImpl : public ContainerSerializationInterface/*{{{*/
{
public:
    explicit ContainerSerializationSTLImpl(Container* container = 0)
    : container_(container)
    {
    }

    // from ContainerSerializationInterface
    std::size_t size() const{
        ASSERT(container_);
        return container_->size();
    }
    std::size_t resize(std::size_t size) const{
        ASSERT(container_);
        container_->resize(size);
        return size;
    }

    void* pointer() const{ return reinterpret_cast<void*>(container_); }
    TypeID type() const{ return TypeID::get<Element>(); }

    bool operator()(Archive& ar, std::size_t index) const{
        ASSERT(container_ && index >= 0 && index < container_->size());
        return ar((*container_)[index]);
    }
    operator bool() const{ return container_ != 0; }
    void serializeNewElement(Archive& ar, const char* name = "") const{
        Element element;
        ar(element, name);
    }
    // ^^^
protected:
    Container* container_;
};/*}}}*/

template<class T>
class ContainerSerializationArrayImpl : public ContainerSerializationInterface{/*{{{*/
    friend class Archive;
public:
    explicit ContainerSerializationArrayImpl(T* array = 0, int size = 0)
    : array_(array)
    , size_(size)
    {
    }

    // from ContainerSerializationInterface:
    std::size_t size() const{ return size_; }
    std::size_t resize(std::size_t size) const{
        return size_;
    }

    void* pointer() const{ return reinterpret_cast<void*>(array_); }
    TypeID type() const{ return TypeID::get<T>(); }

    bool operator()(Archive& ar, std::size_t index) const{
        ASSERT(index < size_);
		return ar(array_[index]);
    }
    operator bool() const{ return array_ != 0; }
    void serializeNewElement(Archive& ar, const char* name) const{
        // XXX do we need this?
        T element;
        ar(element, name);
    }
    // ^^^
    
private:
    T* array_;
	std::size_t size_;
};/*}}}*/

class PointerSerializationInterface
{
public:
	virtual TypeID type() const = 0;
	virtual void create(TypeID type) const = 0;
	virtual TypeID baseType() const = 0;
	virtual Serializer serializer() const = 0;
	virtual void* get() const = 0;
	
	void serialize(Archive& ar);
};

