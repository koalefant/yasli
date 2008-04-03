#pragma once

#include <vector>
#include <map>

#include "yasli/StringList.h"
#include "yasli/API.h"

class Archive;

class YASLI_API EnumDescription{
public:
    int value(const char* name) const;
    const char* name(int value) const;
    const char* nameByIndex(int index) const;
    const char* indexByName(const char* name) const;
    int indexByValue(int value) const;

    bool serialize(Archive& ar, int& value, const char* name) const;

    void add(int value, const char* name);
    const StringListStatic& names() const{ return names_; }
    bool registered() const { return !names_.empty(); }
private:
    StringListStatic names_;

    typedef std::map<const char*, int> NameToValue;
    NameToValue nameToValue_;
    typedef std::map<int, int> ValueToIndex;
    ValueToIndex valueToIndex_;
    typedef std::map<int, const char*> ValueToName;
    ValueToName valueToName_;
};

template<class Enum>
class EnumDescriptionImpl : public EnumDescription{
public: static EnumDescription& the(){
        static EnumDescriptionImpl description;
        return description;
    }
};

template<class Enum>
EnumDescription& getEnumDescription(){
    return EnumDescriptionImpl<Enum>::the();
}

#define SERIALIZATION_ENUM_BEGIN(Type, name)                                        \
    namespace {                                                                     \
        bool registerEnum_##Type();                                                 \
        bool Type##_enum_registered = registerEnum_##Type();                        \
        bool registerEnum_##Type(){                                                 \
            EnumDescription& description = EnumDescriptionImpl<Type>::the();

#define SERIALIZATION_ENUM_BEGIN_NESTED(Class, Enum, name)                          \
    namespace {                                                                     \
	bool registerEnum_##Class##_##Enum();                                           \
		bool Class##_##Enum##_enum_registered = registerEnum_##Class##_##Enum();    \
		bool registerEnum_##Class##_##Enum(){                                       \
			EnumDescription& description = EnumDescriptionImpl<Class::Enum>::the();
                                                                                    
#define SERIALIZATION_ENUM_VALUE(value, name)                                       \
            description.add(int(value), name);                                      
                                                                                   
#define SERIALIZATION_ENUM_END()													\
            return true;                                                            \
        };                                                                          \
    };
