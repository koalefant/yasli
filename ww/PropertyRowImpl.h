#pragma once
#include "PropertyRowField.h"

namespace ww{

struct Unspecified_Derived_Argument {};

template<class Type, class Derived = PropertyRowImpl<Type, Unspecified_Derived_Argument> >
class PropertyRowImpl : public PropertyRowField{
public:
	PropertyRowImpl()
	: PropertyRowField()
	{
	}
	PropertyRowImpl(const char* name, const char* nameAlt, const Type& value, const char* typeName = 0)
	: PropertyRowField(name, nameAlt, typeName ? typeName : typeid(Type).name())
	, value_(value)
	{
	}
	PropertyRowImpl(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
	: PropertyRowField(name, nameAlt, typeName)
	, value_(*reinterpret_cast<Type*>(object))
	{
		ASSERT(sizeof(Type) == size);
	}
	bool assignTo(void* object, size_t size){
		*reinterpret_cast<Type*>(object) = value();
		return true;
	}
	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	void setValue(const Type& value) { value_ = value; }
	Type& value() { return value_; }
	const Type& value() const{ return value_; }

	void serializeValue(Archive& ar){
		ar(value_, "value", "Value");
	}
	PropertyRow* clone() const{
		// если здесь возникла ошибка "error C2440: 'static_cast' : cannot convert from..."
		// скорее всего вы забыли указать Derived аргумент при наследовании от PropertyRowImpl
		return cloneChildren(new Derived((void*)(&value_), sizeof(value_), name_, label_, typeName_), static_cast<const Derived* const>(this));
	}
protected:
	Type value_; 
};

}
