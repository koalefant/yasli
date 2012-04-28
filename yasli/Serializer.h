/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include "yasli/Assert.h"
#include "yasli/TypeID.h"

namespace yasli{

class Archive;

typedef bool(*SerializeStructFunc)(void*, Archive&);

typedef bool(*SerializeContainerFunc)(void*, Archive&, size_t index);
typedef size_t(*ContainerResizeFunc)(void*, size_t size);
typedef size_t(*ContainerSizeFunc)(void*);

// Struct serializer. 
class Serializer{/*{{{*/
	friend class Archive;
public:
	Serializer()
	: object_(0)
	, size_(0)
	, serializeFunc_(0)
	{
	}

	Serializer(TypeID type, void* object, size_t size, SerializeStructFunc serialize)
	: type_(type)
	, object_(object)
	, size_(size)
	, serializeFunc_(serialize)
	{
		YASLI_ASSERT(object != 0);
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
	size_t size() const{ return size_; }

	template<class T>
	static bool serializeRaw(void* rawPointer, Archive& ar){
		YASLI_ESCAPE(rawPointer, return false);
		((T*)(rawPointer))->serialize(ar);
		return true;
	}

	void serialize(Archive& ar) { YASLI_ASSERT(0); }
private:

	TypeID type_;
	void* object_;
	size_t size_;
	SerializeStructFunc serializeFunc_;
};/*}}}*/
typedef std::vector<Serializer> Serializers;

// ---------------------------------------------------------------------------

class ContainerInterface{
public:
	virtual size_t size() const = 0;
	virtual size_t resize(size_t size) = 0;
	virtual bool isFixedSize() const{ return false; }

	virtual void* pointer() const = 0;
	virtual TypeID type() const = 0;
	virtual bool next() = 0;
	virtual void extractInPlacePointers(Archive& ar) const { YASLI_ASSERT(0 && "Not implemented"); }

	virtual void* elementPointer() const = 0;
	virtual size_t elementSize() const = 0;

	virtual bool operator()(Archive& ar, const char* name, const char* label) = 0;
	virtual operator bool() const = 0;
	virtual void serializeNewElement(Archive& ar, const char* name = "", const char* label = 0) const = 0;
};

template<class T>
class ContainerArray : public ContainerInterface{/*{{{*/
	friend class Archive;
public:
	explicit ContainerArray(T* array = 0, int size = 0)
	: array_(array)
	, size_(size)
	, index_(0)
	{
	}

	virtual void extractInPlacePointers(Archive& ar) const
	{
	}

	// from ContainerSerializationInterface:
	size_t size() const{ return size_; }
	size_t resize(size_t size){
		index_ = 0;
		return size_;
	}

	void* pointer() const{ return reinterpret_cast<void*>(array_); }
	TypeID type() const{ return TypeID::get<T>(); }
	void* elementPointer() const { return &array_[index_]; }
	size_t elementSize() const { return sizeof(T); }
	virtual bool isFixedSize() const{ return true; }

	bool operator()(Archive& ar, const char* name, const char* label){
		YASLI_ESCAPE(size_t(index_) < size_, return false);
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
	size_t size_;
};/*}}}*/

class ClassFactoryBase;
class PointerInterface
{
public:
	virtual TypeID type() const = 0;
	virtual void create(TypeID type) const = 0;
	virtual TypeID baseType() const = 0;
	virtual Serializer serializer() const = 0;
	virtual void* get() const = 0;
	virtual ClassFactoryBase* factory() const = 0;
	virtual void extractInPlacePointers(Archive& ar) const{ YASLI_ASSERT(0 && "Not implemented"); }
	
	void serialize(Archive& ar) const;
};

class StringInterface
{
public:
	virtual void set(const char* value) = 0;
	virtual const char* get() const = 0;
	virtual const char** getInplacePointer() const{ return 0; }
};
class WStringInterface
{
public:
	virtual void set(const wchar_t* value) = 0;
	virtual const wchar_t* get() const = 0;
	virtual const wchar_t** getInplacePointer() const{ return 0; }
};

}
// vim:ts=4 sw=4:
