/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <list>
#include "yasli/Archive.h"
#include "yasli/Serializer.h"
#include "ClassFactory.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4512)
#endif
namespace yasli {

template<class Container, class Element>
class ContainerSTL final : public ContainerInterface/*{{{*/
{
public:
	explicit ContainerSTL(Container* container = 0)
	: container_(container)
	, it_(container->begin())
	, size_(container->size())
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
		while(size_t(container_->size()) > _size)
		{
			typename Container::iterator it = container_->end();
			--it;
			container_->erase(it);
		}
		while(size_t(container_->size()) < _size)
			container_->insert(container_->end(), Element());
	}

	// from ContainerInterface
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
	TypeID elementType() const{ return TypeID::get<Element>(); }
	TypeID containerType() const{ return TypeID::get<Container>(); }

	bool next()
	{
		YASLI_ESCAPE(container_ && it_ != container_->end(), return false);
		++it_;
		return it_ != container_->end();
	}

	void* elementPointer() const{ return &*it_; }
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
	operator bool() const{ return container_ != 0; }

	struct ElementInitializer
	{
		Element value;
		// Important to call brackets on constructed value to initialize
		// built-in types to zeros/false.
		ElementInitializer() : value() {}
	};
	void serializeNewElement(Archive& ar, const char* name = "", const char* label = 0) const{
		ElementInitializer element;
		ar(element.value, name, label);
	}
	// ^^^
protected:
	Container* container_;
	typename Container::iterator it_;
	size_t size_;
};/*}}}*/

}

namespace std{

template<class T, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::vector<T, Alloc>& container, const char* name, const char* label)
{
	yasli::ContainerSTL<std::vector<T, Alloc>, T> ser(&container);
	return ar(static_cast<yasli::ContainerInterface&>(ser), name, label);
}

template<class T, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::list<T, Alloc>& container, const char* name, const char* label)
{
	yasli::ContainerSTL<std::list<T, Alloc>, T> ser(&container);
	return ar(static_cast<yasli::ContainerInterface&>(ser), name, label);
}

}

// ---------------------------------------------------------------------------
namespace yasli {

class StringSTL final : public StringInterface
{
public:
	StringSTL(yasli::string& str) : str_(str) { }

	void set(const char* value) { str_ = value; }
	const char* get() const { return str_.c_str(); }
	const void* handle() const { return &str_; }
	TypeID type() const { return TypeID::get<string>(); }
private:
	yasli::string& str_;
};

}

YASLI_STRING_NAMESPACE_BEGIN

inline bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, yasli::string& value, const char* name, const char* label)
{
	yasli::StringSTL str(value);
	return ar(static_cast<yasli::StringInterface&>(str), name, label);
}

YASLI_STRING_NAMESPACE_END

// ---------------------------------------------------------------------------

namespace yasli {

template<class K, class V, class C, class Alloc>
struct STLMap : MapInterface {
	STLMap(std::map<K, V, C, Alloc>& container)
	: container(container)
	, it(container.begin())
	, newIt(container.end())
	{}
	std::map<K, V, C, Alloc>& container;
	typename std::map<K, V, C, Alloc>::iterator newIt;
	typename std::map<K, V, C, Alloc>::iterator it;

	void* pointer() const override{ return &container; }
	TypeID keyType() const override{ return TypeID::get<K>(); }
	TypeID valueType() const override{ return TypeID::get<V>(); }
	TypeID containerType() const override{ return TypeID::get<std::map<K, V, C, Alloc>>(); }
	size_t size() const override{ return container.size(); }
	bool keySerializesToString() const override{ return Traits<K>::serializes_to_string; }

	bool deserializeNewKey(Archive& ar, const char* name, const char* label) override {
		K newKey;
		if (!ar(newKey, name, label))
			return false;
		newIt = container.emplace(std::make_pair(std::move(newKey), V())).first;
		return true;
	}

	bool deserializeNewValue(Archive& ar, const char* name, const char* label) override {
		if (newIt == container.end())
			return false;
		if (!ar(newIt->second, name, label))
			return false;
		newIt = container.end();
		return true;
	}

	bool isEmpty() const override{ return container.empty(); }
	bool next() override{
		++it;
		return it != container.end();
	}
	void serializeKey(Archive& ar, const char* name, const char* label) override {
		YASLI_ASSERT(it != container.end());
		ar(it->first, name, label);
	}
	void serializeValue(Archive& ar, const char* name, const char* label) override {
		YASLI_ASSERT(it != container.end());
		ar(it->second, name, label);
	}
	void serializeDefaultKey(Archive& ar, const char* name, const char* label) override {
		K key;
		ar(key, name, label);
	}
	void serializeDefaultValue(Archive& ar, const char* name, const char* label) override {
		V value;
		ar(value, name, label);
	}

};

}

namespace std {

template<class K, class V, class C, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::map<K, V, C, Alloc>& container, const char* name, const char* label)
{
	if (ar.isInput()) {
		container.clear();
	}
	yasli::STLMap<K, V, C, Alloc> serializer(container);
	return ar(static_cast<yasli::MapInterface&>(serializer), name, label);
}

}
// ---------------------------------------------------------------------------

namespace yasli {

class WStringSTL final : public WStringInterface
{
public:
	WStringSTL(yasli::wstring& str) : str_(str) { }

	void set(const wchar_t* value) { str_ = value; }
	const wchar_t* get() const { return str_.c_str(); }
	const void* handle() const { return &str_; }
	TypeID type() const { return TypeID::get<wstring>(); }
private:
	yasli::wstring& str_;
};

}

YASLI_STRING_NAMESPACE_BEGIN

inline bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, yasli::wstring& value, const char* name, const char* label)
{
	yasli::WStringSTL str(value);
	return ar(static_cast<yasli::WStringInterface&>(str), name, label);
}

YASLI_STRING_NAMESPACE_END

// ---------------------------------------------------------------------------

namespace yasli {

template<class K, class V>
struct StdPair final : std::pair<K, V>
{
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar) 
	{
#if YASLI_STD_PAIR_FIRST_SECOND
		ar(this->first, "first", "^");
		ar(this->second, "second", "^");
#else
		ar(this->first, "key", "^");
		ar(this->second, "value", "^");
#endif
	}
};

}

namespace std{

template<class K, class V>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::pair<K, V>& pair, const char* name, const char* label)
{
	return ar(static_cast<yasli::StdPair<K, V>&>(pair), name, label);
}

}
namespace yasli {

template<class T>
class StdUniquePtr : public PointerInterface
{
public:
	StdUniquePtr(std::unique_ptr<T>& ptr)
	: ptr_(ptr)
	{}
	const char* registeredTypeName() const override{
		if(ptr_)
			return ClassFactory<T>::the().getRegisteredTypeName(ptr_.get());
		else
			return "";
	}
	TypeID pointerType() const override{ return TypeID::get<std::unique_ptr<T> >(); }
	void create(const char* typeName) const override{
		if(typeName && typeName[0] != '\0')
			ptr_.reset(factory()->create(typeName));
		else
			ptr_.reset((T*)0);
	}
	TypeID baseType() const override{ return TypeID::get<T>(); }
	Serializer serializer() const override{ return Serializer(*ptr_); }
	void* get() const override{ return reinterpret_cast<void*>(ptr_.get()); }
	ClassFactory<T>* factory() const override{ return &ClassFactory<T>::the(); }
	const void* handle() const override{ return &ptr_; }
protected:
	std::unique_ptr<T>& ptr_;
};

template<class T, class D>
class StdUniquePtrDeleter : public PointerInterface {
public:
	StdUniquePtrDeleter(std::unique_ptr<T, D>& ptr)
	: ptr_(ptr)
	{}
	const char* registeredTypeName() const override{
		if(ptr_)
			return ClassFactory<T>::the().getRegisteredTypeName(ptr_.get());
		else
			return "";
	}
	TypeID pointerType() const override{ return TypeID::get<std::unique_ptr<T, D> >(); }
	void create(const char* typeName) const override{
		if(typeName && typeName[0] != '\0')
			ptr_.reset(factory()->create(typeName));
		else
			ptr_.reset((T*)0);
	}
	TypeID baseType() const override{ return TypeID::get<T>(); }
	Serializer serializer() const override{ return Serializer(*ptr_); }
	void* get() const override{ return reinterpret_cast<void*>(ptr_.get()); }
	ClassFactory<T>* factory() const override{ return &ClassFactory<T>::the(); }
	const void* handle() const override{ return &ptr_; }
protected:
	std::unique_ptr<T, D>& ptr_;
};

}

namespace std {

template<class T>
bool serialize(yasli::Archive& ar, unique_ptr<T>& ptr, const char* name, const char* label)
{
  yasli::StdUniquePtr<T> serializer(ptr);
  return ar(static_cast<yasli::PointerInterface&>(serializer), name, label);
}

template<class T, class D>
bool serialize(yasli::Archive& ar, unique_ptr<T, D>& ptr, const char* name, const char* label)
{
  yasli::StdUniquePtrDeleter<T, D> serializer(ptr);
  return ar(static_cast<yasli::PointerInterface&>(serializer), name, label);
}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
