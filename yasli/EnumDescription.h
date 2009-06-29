#pragma once

#include <vector>
#include <map>

#include "yasli/ConstString.h"
#include "yasli/StringList.h"
#include "yasli/API.h"
#include "yasli/TypeID.h"

namespace yasli{

class Archive;

class YASLI_API EnumDescription{
public:
    int value(const char* name) const;
    int valueByLabel(const char* label) const;
    const char* name(int value) const;
    const char* label(int value) const;
    const char* nameByIndex(int index) const;
    const char* indexByName(const char* name) const;
    int indexByValue(int value) const;

    bool serialize(Archive& ar, int& value, const char* name, const char* label) const;
    bool serializeBitVector(Archive& ar, int& value, const char* name, const char* label) const;

    void add(int value, const char* name, const char* label = ""); // TODO
    const StringListStatic& names() const{ return names_; }
    const StringListStatic& labels() const{ return labels_; }
    StringListStatic nameCombination(int bitVector) const;
    StringListStatic labelCombination(int bitVector) const;
    bool registered() const { return !names_.empty(); }
	TypeID type() const{ return type_; }
private:
    StringListStatic names_;
    StringListStatic labels_;

    typedef std::map<ConstString, int> NameToValue;
    NameToValue nameToValue_;
    typedef std::map<ConstString, int> LabelToValue;
    LabelToValue labelToValue_;
    typedef std::map<int, int> ValueToIndex;
    ValueToIndex valueToIndex_;
    typedef std::map<int, const char*> ValueToName;
    ValueToName valueToName_;
    typedef std::map<int, const char*> ValueToLabel;
    ValueToName valueToLabel_;
	TypeID type_;
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

}

#define YASLI_ENUM_BEGIN(Type, label)                                                \
    namespace {                                                                     \
        bool registerEnum_##Type();                                                 \
        bool Type##_enum_registered = registerEnum_##Type();                        \
        bool registerEnum_##Type(){                                                 \
		yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Type>::the();

#define YASLI_ENUM_BEGIN_NESTED(Class, Enum, label)                                  \
    namespace {                                                                     \
	bool registerEnum_##Class##_##Enum();                                           \
		bool Class##_##Enum##_enum_registered = registerEnum_##Class##_##Enum();    \
		bool registerEnum_##Class##_##Enum(){                                       \
			yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Class::Enum>::the();
                                                                                    
#define YASLI_ENUM_VALUE(value, label)                                              \
		description.add(int(value), #value, label);                                      
                                                                                   
#define YASLI_ENUM_END()													        \
            return true;                                                            \
        };                                                                          \
    };
