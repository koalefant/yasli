#pragma once

#include "utils/Errors.h"
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

class YASLI_API ContainerSerializer{/*{{{*/
    friend class Archive;
public:
    ContainerSerializer()
    : object_(0)
    , serializeFunc_(0)
    {
    }

    ContainerSerializer(TypeID type, void* object, SerializeContainerFunc serialize, ContainerResizeFunc resize, ContainerSizeFunc size)
    : type_(type)
    , object_(object)
    , serializeFunc_(serialize)
    , resizeFunc_(resize)
    , sizeFunc_(size)
    {
        ASSERT(object);
    }

    template<class T>
    explicit ContainerSerializer(T& object){
        type_ =  TypeID::get<T>();
        object_ = (void*)(&object);
        serializeFunc_ = &ContainerSerializer::serializeRaw<T>;
        resizeFunc_ = &ContainerSerializer::resizeRaw<T>;
        sizeFunc_ = &ContainerSerializer::sizeRaw<T>;
    }

    bool operator()(Archive& ar, std::size_t index) const;
    operator bool() const{ return object_ && serializeFunc_; }

    std::size_t size() const{ ASSERT(sizeFunc_); return sizeFunc_(object_); }
    std::size_t resize(std::size_t size)const{ ASSERT(resizeFunc_); return resizeFunc_(object_, size); }

    void* pointer() const{ return object_; }
    TypeID type() const{ return type_; }
    template<class T>
    static bool serializeRaw(void* rawPointer, Archive& ar, std::size_t index){
        ASSERT(rawPointer);
        T& container = *reinterpret_cast<T*>(rawPointer); 
        return ar(container[index], "");
    }
    template<class T>
    static std::size_t resizeRaw(void* rawPointer, std::size_t size){
        ASSERT(rawPointer);
        T& container = *reinterpret_cast<T*>(rawPointer);
        container.resize(size);
        ASSERT(container.size() == size);
        return size;
    }
    template<int Size>
    static std::size_t resizeArray(void* rawPointer, std::size_t size){
        ASSERT(rawPointer);
        return Size;
    }
    template<class T>
    static std::size_t sizeRaw(void* rawPointer){
        ASSERT(rawPointer);
        return ((T*)(rawPointer))->size();
    }
    template<int Size>
    static std::size_t sizeArray(void* rawPointer){
        ASSERT(rawPointer);
        return Size;
    }
private:

    TypeID type_;
    void* object_;
    SerializeContainerFunc serializeFunc_;
    ContainerResizeFunc resizeFunc_;
    ContainerSizeFunc sizeFunc_;
};/*}}}*/
