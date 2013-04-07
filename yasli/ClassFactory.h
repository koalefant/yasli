/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include <map>
#include <vector>

#include "yasli/Config.h"
#include "yasli/Assert.h"
#include "yasli/ClassFactoryBase.h"
#include "yasli/TypeID.h"

// #ifdef _MSC_VER
// # pragma warning(disable : 4251)
// #endif

namespace yasli{

class Archive;

class TypeDescription{
public:
	TypeDescription(TypeID typeID, const char* name, const char *label, std::size_t size)
	: name_(name)
	, label_(label)
	, size_(size)
	, typeID_(typeID)
	{
#if YASLI_NO_RTTI
		const size_t bufLen = sizeof(typeID.typeInfo_->name);
		strncpy(typeID.typeInfo_->name, name, bufLen - 1);
		typeID.typeInfo_->name[bufLen] = '\0';
#endif
	}
	const char* name() const{ return name_; }
	const char* label() const{ return label_; }
	std::size_t size() const{ return size_; }
	TypeID typeID() const{ return typeID_; }

protected:
	const char* name_;
	const char* label_;
	std::size_t size_;
	TypeID typeID_;
};

class ClassFactoryManager{
public:
	static ClassFactoryManager& the(){
		static ClassFactoryManager factoryManager;
		return factoryManager;
	}

	const ClassFactoryBase* find(TypeID baseType) const{
		Factories::const_iterator it = factories_.find(baseType);
		if(it == factories_.end())
			return 0;
		else
			return it->second;
	}

	void registerFactory(TypeID type, const ClassFactoryBase* factory){
		factories_[type] = factory;
	}
protected:
	typedef std::map<TypeID, const ClassFactoryBase*> Factories;
	Factories factories_;
};

template<class BaseType>
class ClassFactory : public ClassFactoryBase{
public:
	static ClassFactory& the(){
		static ClassFactory factory;
		return factory;
	}

	class CreatorBase{
	public:
		virtual ~CreatorBase() {}
		virtual BaseType* create() const = 0;
		const TypeDescription& description() const{ return *description_; }
#if YASLI_NO_RTTI
		void* vptr() const{ return vptr_; }
#endif
	protected:
		const TypeDescription* description_;
#if YASLI_NO_RTTI
		void* vptr_;
#endif
	};

#if YASLI_NO_RTTI
	static void* extractVPtr(BaseType* ptr)
	{
		return *((void**)ptr);
	}

#endif

	template<class Derived>
	class Creator : public CreatorBase{
	public:
		Creator(const TypeDescription* description){
			this->description_ = description;
#if YASLI_NO_RTTI
			// TODO: remove unnecessary static initialisation
			Derived vptrProbe;
            CreatorBase::vptr_ = extractVPtr(&vptrProbe);
#endif
			ClassFactory::the().registerCreator(this);
		}
		BaseType* create() const{
			return new Derived();
		}
	};

	ClassFactory()
	: ClassFactoryBase(TypeID::get<BaseType>())
	{
		ClassFactoryManager::the().registerFactory(baseType_, this);
	}

	typedef std::map<TypeID, CreatorBase*> TypeToCreatorMap;

	BaseType* create(TypeID derivedType) const
	{
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.find(derivedType);
		if(it != typeToCreatorMap_.end())
			return it->second->create();
		else
			return 0;
	}

	TypeID getTypeID(BaseType* ptr) const
	{
#if YASLI_NO_RTTI
		if (ptr == 0)
			return TypeID();
		void* vptr = extractVPtr(ptr);
        typename VPtrToCreatorMap::const_iterator it = vptrToCreatorMap_.find(vptr);
		if (it == vptrToCreatorMap_.end())
			return TypeID();
		return it->second->description().typeID();
#else
		return TypeID(typeid(*ptr));
#endif
	}

	size_t sizeOf(TypeID derivedType) const
	{
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.find(derivedType);
		if(it != typeToCreatorMap_.end())
			return it->second->description().size();
		else
			return 0;
	}

	BaseType* createByIndex(int index) const
	{
		YASLI_ASSERT(size_t(index) < creators_.size());
		return creators_[index]->create();
	}

	void serializeNewByIndex(Archive& ar, int index, const char* name, const char* label)
	{
		YASLI_ESCAPE(size_t(index) < creators_.size(), return);
		BaseType* ptr = creators_[index]->create();
		ar(*ptr, name, label);
		delete ptr;
	}
	// from ClassFactoryInterface:
	size_t size() const{ return creators_.size(); }
	const TypeDescription* descriptionByIndex(int index) const{
		if(size_t(index) >= int(creators_.size()))
			return 0;
		return &creators_[index]->description();
	}

	const TypeDescription* descriptionByType(TypeID type) const{
		const size_t numCreators = creators_.size();
		for (size_t i = 0; i < numCreators; ++i) {
			if (type == creators_[i]->description().typeID())
				return &creators_[i]->description();
		}
		return 0;
	}

	TypeID findTypeByName(const char* name) const {
		const size_t numCreators = creators_.size();
		for (size_t i = 0; i < numCreators; ++i) {
			const TypeID& typeID = creators_[i]->description().typeID();
			if (strcmp(name, typeID.name()) == 0)
				return typeID;
		}
		return TypeID();
	}
	// ^^^

protected:
	void registerCreator(CreatorBase* creator){
		typeToCreatorMap_[creator->description().typeID()] = creator;
		creators_.push_back(creator);
#if YASLI_NO_RTTI
		vptrToCreatorMap_[creator->vptr()] =  creator;
#endif
	}

	TypeToCreatorMap typeToCreatorMap_;
	std::vector<CreatorBase*> creators_;

#if YASLI_NO_RTTI
	typedef std::map<void*, CreatorBase*> VPtrToCreatorMap;
	VPtrToCreatorMap vptrToCreatorMap_;
#endif
};


class TypeLibrary{
public:
	static TypeLibrary& the();
	const TypeDescription* findByName(const char*) const;
	const TypeDescription* find(TypeID type) const;

	const TypeDescription* registerType(const TypeDescription* info);
protected:
	TypeLibrary();

	typedef std::map<TypeID, const TypeDescription*> TypeToDescriptionMap;
	TypeToDescriptionMap typeToDescriptionMap_;
};

}

#define YASLI_TYPE_NAME(Type, name) \
namespace{ \
	const yasli::TypeDescription Type##_Description(yasli::TypeID::get<Type>(), #Type, name, sizeof(Type)); \
	bool registered_##Type = yasli::TypeLibrary::the().registerType(&Type##_Description) != 0; \
}

#define YASLI_CLASS_NULL(BaseType, name) \
namespace { \
    bool BaseType##_NullRegistered = yasli::ClassFactory<BaseType>::the().setNullLabel(name); \
}

#define YASLI_CLASS(BaseType, Type, name) \
	const yasli::TypeDescription Type##BaseType##_DerivedDescription(yasli::TypeID::get<Type>(), #Type, name, sizeof(Type)); \
	yasli::ClassFactory<BaseType>::Creator<Type> Type##BaseType##_Creator(yasli::TypeLibrary::the().registerType(&Type##BaseType##_DerivedDescription)); \
	int dummyForType_##Type##BaseType;

#define YASLI_FORCE_CLASS(BaseType, Type) \
	extern int dummyForType_##Type##BaseType; \
	int* dummyForTypePtr_##Type##BaseType = &dummyForType_##Type##BaseType + 1;


