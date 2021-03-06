/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "PropertyRow.h"

class PropertyRowContainer;
struct ContainerMenuHandler : PropertyRowMenuHandler
{
public:

	PropertyTree* tree;
	PropertyRowContainer* container;
	PropertyRow* element;
	int pointerIndex;

	ContainerMenuHandler(PropertyTree* tree, PropertyRowContainer* container);

public:
	void onMenuAppendElement();
	void onMenuAppendPointerByIndex();
	void onMenuRemoveAll();
	void onMenuChildInsertBefore();
	void onMenuChildRemove();
};

class PropertyRowContainer : public PropertyRow
{
public:
	PropertyRowContainer();
	bool isContainer() const override{ return true; }
	bool onActivate( PropertyTree* tree, bool force) override;
	bool onContextMenu(IMenu& item, PropertyTree* tree) override;
	void redraw(IDrawContext& context) override;
	bool onKeyDown(PropertyTree* tree, const KeyEvent* key) override;

	void labelChanged() override;
	bool isStatic() const override{ return false; }
	bool isSelectable() const override{ return userWidgetSize() == 0 ? false : true; }

	PropertyRow* defaultRow(PropertyTreeModel* model);
	const PropertyRow* defaultRow(const PropertyTreeModel* model) const;
	void serializeValue(yasli::Archive& ar) override;

	const char* elementTypeName() const{ return elementTypeName_; }
	virtual void setValueAndContext(const yasli::ContainerInterface& value, yasli::Archive& ar) {
		fixedSize_ = value.isFixedSize();
		elementTypeName_ = value.elementType().name();
	}
	const char* typeNameForFilter(PropertyTree* tree) const override;
	yasli::string valueAsString() const override;
	// C-array is an example of fixed size container
    bool isFixedSize() const { return fixedSize_; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_AFTER_NAME; }
	int widgetSizeMin(const PropertyTree*) const override{ return userWidgetSize() >=0 ? userWidgetSize() : 42; }

protected:
	void generateMenu(IMenu& menu, PropertyTree* tree);

	bool fixedSize_;
	const char* elementTypeName_;
	char buttonLabel_[8];
};
