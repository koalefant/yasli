/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Serializer.h"

namespace yasli {

template<class Container, class Element>
class ContainerSTL : public ContainerInterface/*{{{*/
{
public:
	explicit ContainerSTL(Container* container = 0)
	: container_(container)
	, size_(container->size())
	, it_(container->begin())
	{
		YASLI_ASSERT(container_ != 0);
	}

	template<class T, class A>
	void resizeHelper(size_t _size, std::vector<T, A>& _v) const
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

	void extractInPlacePointersHelper(Archive& ar, ...) const
	{
		YASLI_ASSERT(0 && "Container is not supported");
	}

	template<class T, class A> 
	void extractInPlacePointersHelper(Archive& ar, std::vector<T, A>* _v) const
	{
#ifdef _MSC_VER
#pragma pack(push,_CRT_PACKING)
		struct MSVector {
			char padding[8];
			T* ptr1;
			T* ptr2;
			T* ptr3;
		};
#pragma pack(pop)
		MSVector* v = (MSVector*)_v;
		ar.inPlacePointer((void**)&v->ptr1, 0);
		ar.inPlacePointer((void**)&v->ptr2, (v->ptr2 - v->ptr1) * sizeof(T));
		ar.inPlacePointer((void**)&v->ptr3, (v->ptr3 - v->ptr1) * sizeof(T));
#else
    YASLI_ASSERT(0 && "Unsupported platform");
// # warning Unsupported platform
#endif
	}

	// from ContainerSerializationInterface
	size_t size() const{
		YASLI_ESCAPE(container_ != 0, return 0);
		return container_->size();
	}
	size_t resize(size_t size){
		YASLI_ESCAPE(container_ != 0, return 0);
		resizeHelper(size, container_);
		it_ = container_->begin();
		size_ = size;
		return size;
	}

	void* pointer() const{ return reinterpret_cast<void*>(container_); }
	TypeID type() const{ return TypeID::get<Element>(); }

	bool next()
	{
		YASLI_ESCAPE(container_ && it_ != container_->end(), return false);
		++it_;
		return it_ != container_->end();
	}

	void* elementPointer() const { return &*it_; }
  size_t elementSize() const { return sizeof(typename Container::value_type); }

	bool operator()(Archive& ar, const char* name, const char* label){
		YASLI_ESCAPE(container_, return false);
		if(it_ == container_->end())
		{
			it_ = container_->insert(container_->end(), Element());
			return ar(*it_, name, label);
		}
		else
			return ar(*it_, name, label);
	}
	void extractInPlacePointers(Archive& ar) const
	{
		extractInPlacePointersHelper(ar, container_);
	}
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

}

template<class T, class Alloc>
bool serialize(yasli::Archive& ar, std::vector<T, Alloc>& container, const char* name, const char* label)
{
	yasli::ContainerSTL<std::vector<T, Alloc>, T> ser(&container);
	return ar(static_cast<yasli::ContainerInterface&>(ser), name, label);
}

template<class T, class Alloc>
bool serialize(yasli::Archive& ar, std::list<T, Alloc>& container, const char* name, const char* label)
{
	yasli::ContainerSTL<std::list<T, Alloc>, T> ser(&container);
	return ar(static_cast<yasli::ContainerInterface&>(ser), name, label);
}

// ---------------------------------------------------------------------------
namespace yasli {

class StringSTL : public StringInterface
{
public:
	StringSTL(std::string& str) : str_(str) { }

	void set(const char* value) { str_ = value; }
	const char* get() const { return str_.c_str(); }
	const char** getInplacePointer() const
	{
#ifdef _MSC_VER
		bool usesInternalBuffer = str_.c_str() >= (const char*)&str_ && str_.c_str() < (const char*)&str_ + sizeof(str_);
		if (!usesInternalBuffer)
			return ((const char**)&str_) + 2;
#else
		YASLI_ASSERT(0 && "Unsupported platform");
#endif
		return 0;
	}
private:
	std::string& str_;
};

}

inline bool serialize(yasli::Archive& ar, std::string& value, const char* name, const char* label)
{
	yasli::StringSTL str(value);
	return ar(static_cast<yasli::StringInterface&>(str), name, label);
}

// ---------------------------------------------------------------------------

namespace yasli {

class WStringSTL : public WStringInterface
{
public:
	WStringSTL(std::wstring& str) : str_(str) { }

	void set(const wchar_t* value) { str_ = value; }
	const wchar_t* get() const { return str_.c_str(); }
private:
	std::wstring& str_;
};

}

inline bool serialize(yasli::Archive& ar, std::wstring& value, const char* name, const char* label)
{
	yasli::WStringSTL str(value);
	return ar(static_cast<yasli::WStringInterface&>(str), name, label);
}
