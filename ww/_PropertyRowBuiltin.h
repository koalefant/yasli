#pragma once

#include <string>
#include "ww/API.h"
#include "ww/PropertyRowImpl.h"
#include "ww/Entry.h"
#include "ww/Win32/Drawing.h"
#include "yasli/MemoryWriter.h"

namespace yasli{
class EnumDescription;
}

namespace ww{
using std::string;
using std::wstring;

class PropertyTreeModel;

class PropertyRowContainer : public PropertyRow, public sigslot::has_slots{
public:
	enum { Custom = false };
	PropertyRowContainer(const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, bool readOnly);
	bool isContainer() const{ return true; }
	bool onActivate( PropertyTree* tree, bool force);
	bool onContextMenu(PopupMenuItem& item, PropertyTree* tree);
	void redraw(const PropertyDrawContext& context);
	bool onKeyDown(PropertyTree* tree, KeyPress key);

	void onMenuAppendElement(PropertyTree* tree);
	void onMenuAppendPointerByIndex(int index, PropertyTree* model);
	void onMenuClear(PropertyTreeModel* model);

	void onMenuChildInsertBefore(PropertyRow* child, PropertyTree* model);
	void onMenuChildRemove(PropertyRow* child, PropertyTreeModel* model);

	PropertyRow* clone() const{
		return cloneChildren(new PropertyRowContainer(name_, label_, typeName_, elementTypeName_, readOnly_), this);
	}
	void digestReset();
	bool isStatic() const{ return false; }

	PropertyRow* defaultRow(PropertyTreeModel* model);
	const PropertyRow* defaultRow(const PropertyTreeModel* model) const;
	void serializeValue(Archive& ar);

	const char* elementTypeName() const{ return elementTypeName_; }
    std::string valueAsString() const;
	// C-array is an example of fixed size container
	bool isFixedSize() const{ return fixedSize_; }
	bool hasWidgetAt(WidgetPosition pos) const{
		return pos == WIDGET_POS_AFTER_NAME;
	}

protected:
	void generateMenu(PopupMenuItem& root, PropertyTree* tree);

	bool fixedSize_;
	const char* elementTypeName_;
	wchar_t buttonLabel_[8];
};

class PropertyRowEnum : public PropertyRow{
public:
	enum{ Custom = false };
	PropertyRowEnum(const char* name = "", const char* nameAlt = "", int value = 0, const EnumDescription* descriptor = 0);
	int value() const;
	void setValue(int index);
	void setVisibleIndex(int index);
	bool assignTo(void* object, size_t size);
	bool isStatic() const{ return false; }
	const EnumDescription* descriptor() { return descriptor_; }
	PropertyRowWidget* createWidget(PropertyTree* tree);

	PropertyRow* clone() const{ return new PropertyRowEnum(name_, label_, value_, descriptor_);	}

	void serializeValue(Archive& ar);
	std::string valueAsString() const;

protected:
	int value_;
	const EnumDescription* descriptor_;
};

class PropertyRowPointer : public PropertyRow, public sigslot::has_slots{
public:
	enum { Custom = false };
	PropertyRowPointer();
	PropertyRowPointer(const char* name, const char* label, const PointerSerializationInterface &ptr);
	PropertyRowPointer(const char* name, const char* label, TypeID baseType, TypeID derivedType, ClassFactoryBase* factory);

	bool assignTo(const PointerSerializationInterface &ptr);

	TypeID baseType() const{ return baseType_; }
	TypeID derivedType() const{ return derivedType_; }
	ClassFactoryBase* factory() const{ return factory_; }
    bool onActivate( PropertyTree* tree, bool force);
    bool onMouseDown(PropertyTree* tree, Vect2 point, bool& changed);
	bool onContextMenu(PopupMenuItem &root, PropertyTree* tree);
	void onMenuCreateByIndex(int index, bool useDefaultValue, PropertyTree* tree);
	bool isStatic() const{ return false; }
	bool isPointer() const{ return true; }
    int widgetSizeMin() const;
    std::wstring generateLabel() const;
	std::string valueAsString() const;
	PropertyRow* clone() const{
		return cloneChildren(new PropertyRowPointer(name_, label_, baseType_, derivedType_, factory_), this);
	}
	void redraw(const PropertyDrawContext& context);
	void serializeValue(Archive& ar);
protected:
	void updateTitle();

    TypeID baseType_;
    TypeID derivedType_;
    ClassFactoryBase* factory_;

	std::string title_;
};


class PropertyRowBool : public PropertyRow{
public:
	enum { Custom = false };
	PropertyRowBool(const char* name = "", const char* nameAlt = "", bool value = false);
	bool assignTo(void* val, size_t size);

	void redraw(const PropertyDrawContext& context);
    bool isLeaf() const{ return true; }
    bool isStatic() const{ return false; }

	bool onActivate(PropertyTree* tree, bool force);
	void digestReset();
	string valueAsString() const { return value_ ? labelUndecorated() : ""; }
	wstring searchValue() const{ return value_ ? L"true" : L"false"; }
	bool hasWidgetAt(WidgetPosition pos) const{
		return pos == WIDGET_POS_ICON;
	}
	PropertyRow* clone() const{
		return cloneChildren(new PropertyRowBool(name_, label_, value_), this);
	}
    void serializeValue(Archive& ar);
protected:
    bool value_;
};

class PropertyRowString : public PropertyRowImpl<std::wstring, PropertyRowString>{
public:
	enum { Custom = false };
	PropertyRowString(const char* name = "", const char* nameAlt = "", const std::wstring& value = std::wstring());
	PropertyRowString(const char* name, const char* nameAlt, const std::string& value);
	PropertyRowString(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName); // понадобился из за PropertyRowImpl
	bool assignTo(std::string& str);
	bool assignTo(std::wstring& str);
	PropertyRowWidget* createWidget(PropertyTree* tree);
	std::string valueAsString() const;
	std::wstring valueAsWString() const { return value_; }
};

class PropertyRowStringListValue : public PropertyRowImpl<StringListValue, PropertyRowStringListValue>{
public:
	enum { Custom = true };
	PropertyRowStringListValue(const char* name = "", const char* nameAlt = "", const StringListValue& value = StringListValue());
	PropertyRowStringListValue(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName); // понадобился из за PropertyRowImpl

	// virtuals:
	PropertyRowWidget* createWidget(PropertyTree* tree);
	std::string valueAsString() const { return value_.c_str(); }
	bool assignTo(void* object, size_t size){
		*reinterpret_cast<StringListValue*>(object) = value().c_str();
		return true;
	}
};

class PropertyRowStringListStaticValue : public PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>{
public:
	enum { Custom = false };
	PropertyRowStringListStaticValue(const char* name = "", const char* nameAlt = "", const StringListStaticValue& value = StringListStaticValue());
	PropertyRowStringListStaticValue(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName); // понадобился из за PropertyRowImpl

	// virtuals:
	PropertyRowWidget* createWidget(PropertyTree* tree);
	std::string valueAsString() const { return value_.c_str(); }
	bool assignTo(void* object, size_t size){
		*reinterpret_cast<StringListStaticValue*>(object) = value().index();
		return true;
	}
};

// используется widget-ом редактриования
class PropertyRowNumericInterface{
public:
	virtual bool setValueFromString(const char* str) = 0;
	virtual std::string valueAsString() const = 0;
};


class PropertyRowWidgetNumeric : public PropertyRowWidget, public sigslot::has_slots{
public:
    PropertyRowWidgetNumeric(PropertyRow* row, PropertyTreeModel* mode, PropertyRowNumericInterface * numeric, PropertyTree* tree);
	~PropertyRowWidgetNumeric(){}

	void onChange();
	void commit();

	Widget* actualWidget() { return entry_; }
protected:
	SharedPtr<Entry> entry_;
	PropertyRowNumericInterface* numeric_;
	PropertyTree* tree_;
};


template<class T>
std::string numericAsString(T value)
{
	MemoryWriter buf;
	buf << value;
	return buf.c_str();
}



template<class Derived, class Default>
struct SelectNumericDerived{
	typedef Derived Type;
};

template<class Default>
struct SelectNumericDerived<Unspecified_Derived_Argument, Default>{
	typedef Default Type;
};

template<class Type, class _Derived = Unspecified_Derived_Argument >
class PropertyRowNumeric : public PropertyRowImpl<Type, typename SelectNumericDerived<_Derived, PropertyRowNumeric<Type, _Derived> >::Type>, public PropertyRowNumericInterface{
public:
	enum { Custom = false };
	typedef typename SelectNumericDerived<_Derived, PropertyRowNumeric>::Type Derived;
	PropertyRowNumeric(const char* name = "", const char* nameAlt = "", Type value = Type())
	: PropertyRowImpl<Type, Derived>((void*)(&value), sizeof(Type), name, nameAlt, typeid(Type).name())
	{
		widgetSizeMin_ = 60;
	}
	PropertyRowNumeric(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
	: PropertyRowImpl<Type, Derived>(object, size, name, nameAlt, typeName)
	{
		widgetSizeMin_ = 60;
	}
	PropertyRowWidget* createWidget(PropertyTree* tree){
		return new PropertyRowWidgetNumeric(this, tree->model(), this, tree);
	}

	bool setValueFromString(const char* str){
		Type value = value_;
		value_ = Type(atof(str));
		return value_ != value;
	}
	std::string valueAsString() const{ 
		return numericAsString(Type(value_)); 
	}
};

}

