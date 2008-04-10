#pragma once
#include <map>

#include "yasli/Assert.h"
#include "yasli/TypeID.h"
#include "yasli/API.h"

#ifdef _MSC_VER
# pragma warning(disable : 4251)
#endif


class Archive;

class YASLI_API TypeDescription{
public:
    TypeDescription(TypeID typeID, const char* name, std::size_t size)
    : name_(name)
    , size_(size)
    , typeID_(typeID)
    {
    }
    const char* name() const{ return name_; }
    std::size_t size() const{ return size_; }
    TypeID typeID() const{ return typeID_; }

protected:
    const char* name_;
    std::size_t size_;
    TypeID typeID_;
};


class ClassFactoryBase{
public: 
	ClassFactoryBase(TypeID baseType)
	: baseType_(baseType)
	{
	}

	virtual int size() const = 0;
	virtual const TypeDescription* descriptionByIndex(int index) const = 0;	
protected:
	TypeID baseType_;

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
    protected:
        const TypeDescription* description_;
    };

    template<class Derived>
    class Creator : public CreatorBase{
    public:
        Creator(const TypeDescription* description){
            this->description_ = description;
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

    BaseType* createByIndex(int index) const
    {
        ASSERT(index >= 0 && index < typeToCreatorMap_.size());
        typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.begin();
        while(index--)
            ++it;
        return it->second->create();
    }
	// from ClassFactoryInterface:
	int size() const{ return typeToCreatorMap_.size(); }
	const TypeDescription* descriptionByIndex(int index) const{
		if(index < 0 || index >= int(typeToCreatorMap_.size()))
			return 0;
		TypeToCreatorMap::const_iterator it = typeToCreatorMap_.begin();
		std::advance(it, index);
		return &it->second->description();
	}

	// ^^^

protected:
    void registerCreator(CreatorBase* creator){
        typeToCreatorMap_[creator->description().typeID()] = creator;
    }

    TypeToCreatorMap typeToCreatorMap_;
};


class YASLI_API TypeLibrary{
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


#define SERIALIZATION_TYPE(Type, name) \
namespace{ \
    const TypeDescription Type##_Description(TypeID::get<Type>(), name, sizeof(Type)); \
    bool registered_##Type = TypeLibrary::the().registerType(&Type##_Description) != 0; \
}

#define SERIALIZATION_DERIVED_TYPE(BaseType, Type, name) \
    const TypeDescription Type##_DerivedDescription(TypeID::get<Type>(), name, sizeof(Type)); \
    ClassFactory<BaseType>::Creator<Type> Type##_Creator(TypeLibrary::the().registerType(&Type##_DerivedDescription)); \
	int dummyForType_##Type;

#define SERIALIZATION_FORCE_DERIVED_TYPE(BaseType, Type) \
	extern int dummyForType_##Type; \
	int* dummyForTypePtr_##Type = &dummyForType_##Type + 1;
