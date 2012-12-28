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
#include "ww/PropertyRowImpl.h"
#include "ww/Entry.h"
#include "ww/Win32/Drawing.h"
#include "ww/Unicode.h"

namespace yasli{
class EnumDescription;
}

namespace ww{

class PropertyTreeModel;

class PropertyRowContainer : public PropertyRow, public has_slots{
public:
	enum { Custom = false };
	PropertyRowContainer(const char* name, const char* nameAlt, const char* typeName, const char* elementTypeName, bool fixedSizeContainer);
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
		return cloneChildren(new PropertyRowContainer(name_, label_, typeName_, elementTypeName_, userReadOnly_), this);
	}
	void digestReset(const PropertyTree* tree);
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return userWidgetSize() == 0 ? false : true; }

	PropertyRow* defaultRow(PropertyTreeModel* model);
	const PropertyRow* defaultRow(const PropertyTreeModel* model) const;
	void serializeValue(Archive& ar);

	const char* elementTypeName() const{ return elementTypeName_; }
    std::string valueAsString() const;
	// C-array is an example of fixed size container
	bool isFixedSize() const{ return fixedSize_; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_AFTER_NAME; }
	int widgetSizeMin() const{ return userWidgetSize() >=0 ? userWidgetSize() : 36; }

protected:
	void generateMenu(PopupMenuItem& root, PropertyTree* tree);

	bool fixedSize_;
	const char* elementTypeName_;
	wchar_t buttonLabel_[8];
};

}

