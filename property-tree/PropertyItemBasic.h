#pragma once
#include <algorithm>
#include "yasli/Serializer.h"
#include "yasli/StringList.h"
#include "PropertyItem.h"

class PropertyItemStruct : public PropertyItem{
public:
    PropertyItemStruct()
    : PropertyItem("")
    {
    }

    PropertyItemStruct(const PropertyItemStruct& original)
    : PropertyItem(original)
    {
    }
    PropertyItemStruct(const char* name, const Serializer& zer)
    : PropertyItem(name, zer.type())
    {
    }
    bool isLeaf() const{ return false; }

	PROPERTY_ITEM_CLONE_IMPL(PropertyItemStruct)
};

class PropertyItemBool : public PropertyItemCheck{
public:
    PropertyItemBool(const char* name, bool value)
    : PropertyItemCheck(name, TypeID::get<bool>())
    , value_(value)
    {
		addDefaultCheckStates();
    }
    PropertyItemBool(const PropertyItemBool& original)
    : PropertyItemCheck(original)
    , value_(original.value_)
    {
    }
	bool activate(const ViewContext& context);

	int stateIndex() const{ return value_ ? 1 : 0; }
    PROPERTY_ITEM_CLONE_IMPL(PropertyItemBool)
	PROPERTY_ITEM_ASSIGN_IMPL(bool)
protected:
    bool value_;
};

class PropertyItemString : public PropertyItemField{
public:
    PropertyItemString(const char* name = "", const std::string& value = std::string())
    : PropertyItemField(name, TypeID::get<std::string>())
    , value_(value)
    {
    }
    PropertyItemString(const PropertyItemString& original)
    : PropertyItemField(original)
    , value_(original.value_)
    {
    }
	std::string toString() const{ return value_; }
    void fromString(const char* str){ value_ = str; }

	PROPERTY_ITEM_CLONE_IMPL(PropertyItemString)
	PROPERTY_ITEM_ASSIGN_IMPL(std::string)
protected:
    std::string value_;
};

class PropertyItemFloat : public PropertyItemField{
public:
    PropertyItemFloat(const char* name = "", float value = 0.0f)
    : PropertyItemField(name, TypeID::get<std::string>())
    , value_(value)
    {
    }
    PropertyItemFloat(const PropertyItemFloat& original)
    : PropertyItemField(original)
    , value_(original.value_)
    {
    }
	std::string toString() const{
        char buf[60];
        int offset = sprintf(buf, "%5f", value_);
        char* last = buf + std::min(int(sizeof(buf)), offset) - 1;
        while(*last == '0')
            --last;
		return std::string(buf, last + 1);
    }
    void fromString(const char* str){
        value_ = atof(str);
    }
	PROPERTY_ITEM_CLONE_IMPL(PropertyItemFloat)
	PROPERTY_ITEM_ASSIGN_IMPL(float)
protected:
    float value_;
};

class PropertyItemInt : public PropertyItemField{
public:
    PropertyItemInt(const char* name = "", int value = 0)
    : PropertyItemField(name, TypeID::get<int>())
    , value_(value)
    {
    }
    PropertyItemInt(const PropertyItemInt& original)
    : PropertyItemField(original)
    , value_(original.value_)
    {
    }
	std::string toString() const{
		char buf[60];
		sprintf(buf, "%i", value_);
		return buf;
	}
    void fromString(const char* str){
		value_ = atoi(str);
	}
    PROPERTY_ITEM_CLONE_IMPL(PropertyItemInt)
	PROPERTY_ITEM_ASSIGN_IMPL(int)
protected:
    int value_;
};

class PropertyItemLong : public PropertyItemField{
public:
    PropertyItemLong(const char* name = "", long value = 0)
    : PropertyItemField(name, TypeID::get<long>())
    , value_(value)
    {
    }
    PropertyItemLong(const PropertyItemLong& original)
    : PropertyItemField(original)
    , value_(original.value_)
    {
    }
	std::string toString() const{
		char buf[60];
		sprintf(buf, "%i", value_);
		return buf;
	}
    void fromString(const char* str){
		value_ = atoi(str);
	}
	PROPERTY_ITEM_CLONE_IMPL(PropertyItemLong)
	PROPERTY_ITEM_ASSIGN_IMPL(long)
protected:
    long value_;
};

class PropertyItemStringListBase : public PropertyItemField{
public:
    PropertyItemStringListBase(const char* name);
    PropertyItemStringListBase(const char* name, TypeID type);
    PropertyItemStringListBase(const PropertyItemStringListBase& original)
    : PropertyItemField(original)
    {
    }
    PropertyControl* createControl(const ViewContext& context);
    virtual void getStringList(StringList& out) const = 0;
    virtual int index() const = 0;
};

class PropertyItemStringListStatic : public PropertyItemStringListBase{
public:
    PropertyItemStringListStatic()
    : PropertyItemStringListBase("") {}


    PropertyItemStringListStatic(const char* name, const Serializer& zer)
    : PropertyItemStringListBase(name, zer.type())
    , value_(*reinterpret_cast<StringListStaticValue*>(zer.pointer()))
    {
        ASSERT(zer.type() == TypeID::get<StringListStaticValue>());
    }

    PropertyItemStringListStatic(const PropertyItemStringListStatic& original)
    : PropertyItemStringListBase(original)
    , value_(original.value_)
    {

    }

    std::string toString() const;
    void fromString(const char* str);

    const StringListStaticValue& value() const{ return value_; }
    void getStringList(StringList& out) const{ out = value_.stringList(); }
    int index() const{ return value_.index(); }
    PROPERTY_ITEM_CLONE_IMPL(PropertyItemStringListStatic)
	PROPERTY_ITEM_ASSIGN_IMPL(StringListStaticValue)
protected:
    StringListStaticValue value_;
};


class PropertyItemStringList : public PropertyItemStringListBase{
public:
    PropertyItemStringList()
    : PropertyItemStringListBase("") {}

    PropertyItemStringList(const char* name, TypeID type)
    : PropertyItemStringListBase(name, type)
    {
    }

	PropertyItemStringList(const char* name, const Serializer& zer)
    : PropertyItemStringListBase(name, zer.type())
    , value_(*reinterpret_cast<StringListValue*>(zer.pointer()))
    {
        ASSERT(zer.type() == TypeID::get<StringListValue>());
    }

    PropertyItemStringList(const PropertyItemStringList& original)
    : PropertyItemStringListBase(original)
    , value_(original.value_)
    {
    }

	std::string toString() const;
    void fromString(const char* str);


    const StringListValue& value() const{ return value_; }
    void getStringList(StringList& out) const{ out = value_.stringList(); }
    int index() const{ return value_.index(); }
    PROPERTY_ITEM_CLONE_IMPL(PropertyItemStringList)
	PROPERTY_ITEM_ASSIGN_IMPL(StringListValue)
protected:
    StringListValue value_;
};
