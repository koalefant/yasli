#pragma once

#include <vector>
#include "yasli/Assert.h"
#include "yasli/API.h"
#include "yasli/TypeID.h"

namespace yasli{

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

	Serializer(const Serializer& _original)
	: type_(_original.type_)
	, object_(_original.object_)
	, size_(_original.size_)
	, serializeFunc_(_original.serializeFunc_)
	{
	}

    template<class T>
    explicit Serializer(T& object){
        type_=  TypeID::get<T>();
        object_ = (void*)(&object);
		size_ = sizeof(T);
        serializeFunc_ = &Serializer::serializeRaw<T>;
    }

    template<class T>
    explicit Serializer(T& object, TypeID type){
        type_ =  type;
        object_ = (void*)(&object);
		size_ = sizeof(T);
        serializeFunc_ = &Serializer::serializeRaw<T>;
    }

    bool operator()(Archive& ar, const char* name, const char* label) const;
    bool operator()(Archive& ar) const;
    operator bool() const{ return object_ && serializeFunc_; }
    void* pointer() const{ return object_; }
    TypeID type() const{ return type_; }
	std::size_t size() const{ return size_; }

    template<class T>
    static bool serializeRaw(void* rawPointer, Archive& ar){
        ESCAPE(rawPointer, return false);
        ((T*)(rawPointer))->serialize(ar);
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
    virtual std::size_t resize(std::size_t size) = 0;

    virtual void* pointer() const = 0;
    virtual TypeID type() const = 0;
    virtual bool next() = 0;

    virtual bool operator()(Archive& ar, const char* name, const char* label) = 0;
    virtual operator bool() const = 0;
    virtual void serializeNewElement(Archive& ar, const char* name = "", const char* label = 0) const = 0;
};

template<class Container, class Element>
class ContainerSerializationSTLImpl : public ContainerSerializationInterface/*{{{*/
{
public:
    explicit ContainerSerializationSTLImpl(Container* container = 0)
    : container_(container)
    , size_(container->size())
    , it_(container->begin())
    {
        ASSERT(container_);
    }

    template<class T, class A> const
    void resizeHelper(size_t _size, std::vector<T, A>& _v)
    {
        _v.resize( _size );
    }

    void resizeHelper(size_t _size, ...) const
    {
        while(container_->size() > _size)
        {
            typename Container::iterator it = container_->end();
            --it;
            container_->erase(it);
        }
        while(container_->size() < _size)
            container_->insert(container_->end(), Element());
    }

    // from ContainerSerializationInterface
    std::size_t size() const{
        ESCAPE(container_ != 0, return 0);
        return container_->size();
    }
    std::size_t resize(std::size_t size){
        ESCAPE(container_ != 0, return 0);
        resizeHelper(size, container_);
        it_ = container_->begin();
        size_ = size;
        return size;
    }

    void* pointer() const{ return reinterpret_cast<void*>(container_); }
    TypeID type() const{ return TypeID::get<Element>(); }

    bool next()
    {
        ESCAPE(container_ && it_ != container_->end(), return false);
        ++it_;
        return it_ != container_->end();
    }

    bool operator()(Archive& ar, const char* name, const char* label){
        ESCAPE(container_, return false);
        if(it_ == container_->end())
		{
			it_ = container_->insert(container_->end(), Element());
			return ar(*it_, name, label);
		}
		else
			return ar(*it_, name, label);
    }
    /*
    bool operator()(Archive& ar, std::size_t index) const{
        CHECK(container_ && size_t(index) < container_->size());
        return ar((*container_)[index]);
    }
    */
    operator bool() const{ return container_ != 0; }
    void serializeNewElement(Archive& ar, const char* name = "", const char* label = 0) const{
        Element element;
        ar(element, name, label);
    }
    // ^^^
protected:
    Container* container_;
    typename Container::iterator it_;
    size_t size_;
};/*}}}*/

template<class T>
class ContainerSerializationArrayImpl : public ContainerSerializationInterface{/*{{{*/
    friend class Archive;
public:
    explicit ContainerSerializationArrayImpl(T* array = 0, int size = 0)
    : array_(array)
    , size_(size)
    , index_(0)
    {
    }

    // from ContainerSerializationInterface:
    std::size_t size() const{ return size_; }
    std::size_t resize(std::size_t size){
        index_ = 0;
        return size_;
    }

    void* pointer() const{ return reinterpret_cast<void*>(array_); }
    TypeID type() const{ return TypeID::get<T>(); }

    bool operator()(Archive& ar, const char* name, const char* label){
        ESCAPE(size_t(index_) < size_, return false);
		return ar(array_[index_], name, label);
    }
    operator bool() const{ return array_ != 0; }
    bool next(){
        ++index_;
        return size_t(index_) < size_;
    }
    void serializeNewElement(Archive& ar, const char* name, const char* label) const{
        T element;
        ar(element, name, label);
    }
    // ^^^
    
private:
    T* array_;
    int index_;
	std::size_t size_;
};/*}}}*/

class ClassFactoryBase;
class PointerSerializationInterface
{
public:
	virtual TypeID type() const = 0;
	virtual void create(TypeID type) const = 0;
	virtual TypeID baseType() const = 0;
	virtual Serializer serializer() const = 0;
	virtual void* get() const = 0;
	virtual ClassFactoryBase* factory() const = 0;
	
	void serialize(Archive& ar) const;
};

typedef std::vector<Serializer> Serializers;

}
