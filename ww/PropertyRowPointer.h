/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Strings.h"
#include "ww/API.h"
#include "ww/PropertyRow.h"

namespace ww{

class PropertyTreeModel;

class PropertyRowPointer : public PropertyRow, public has_slots{
public:
	enum { Custom = false };
	PropertyRowPointer();

	bool assignTo(yasli::PointerInterface &ptr);
	void setValue(const yasli::PointerInterface& ptr);
	using PropertyRow::assignTo;

	yasli::TypeID baseType() const{ return baseType_; }
	const char* derivedTypeName() const{ return derivedTypeName_.c_str(); }
	yasli::TypeID getDerivedType(yasli::ClassFactoryBase* factory) const;
	void setDerivedType(const yasli::TypeID& typeID, yasli::ClassFactoryBase* factory);
	yasli::ClassFactoryBase* factory() const{ return factory_; }
    bool onActivate( PropertyTree* tree, bool force);
    bool onMouseDown(PropertyTree* tree, Vect2 point, bool& changed);
	bool onContextMenu(PopupMenuItem &root, PropertyTree* tree);
	void onMenuCreateByIndex(int index, bool useDefaultValue, PropertyTree* tree);
	bool isStatic() const{ return false; }
	bool isPointer() const{ return true; }
    int widgetSizeMin() const;
    string generateLabel() const;
	string valueAsString() const;
	PropertyRow* clone() const{
		PropertyRowPointer* result = new PropertyRowPointer();
		result->setNames(name_, label_, typeName_);
		result->baseType_ = baseType_;
		result->factory_ = factory_;
		result->derivedTypeName_ = derivedTypeName_;
		result->derivedLabel_ = derivedLabel_;
		return cloneChildren(result, this);
	}
	void redraw(const PropertyDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	void serializeValue(Archive& ar);
protected:

	yasli::TypeID baseType_;
	yasli::string derivedTypeName_;
	yasli::string derivedLabel_;

	// this member is available for instances deserialized from clipboard:
	yasli::ClassFactoryBase* factory_;
};

}

