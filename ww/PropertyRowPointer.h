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

namespace yasli{
class EnumDescription;
}

namespace ww{

class PropertyTreeModel;

class PropertyRowPointer : public PropertyRow, public has_slots{
public:
	enum { Custom = false };
	PropertyRowPointer();
	PropertyRowPointer(const char* name, const char* label, const PointerInterface &ptr);
	PropertyRowPointer(const char* name, const char* label, TypeID baseType, TypeID derivedType, ClassFactoryBase* factory);

	bool assignTo(PointerInterface &ptr);

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
    wstring generateLabel() const;
	string valueAsString() const;
	PropertyRow* clone() const{
		return cloneChildren(new PropertyRowPointer(name_, label_, baseType_, derivedType_, factory_), this);
	}
	void redraw(const PropertyDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	void serializeValue(Archive& ar);
protected:
    TypeID baseType_;
    TypeID derivedType_;
    ClassFactoryBase* factory_;

	string title_;
};

}

