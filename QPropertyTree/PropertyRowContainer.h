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
	Q_OBJECT
public:

	QPropertyTree* tree;
	PropertyRowContainer* container;
	PropertyRow* element;
	int pointerIndex;

	ContainerMenuHandler(QPropertyTree* tree, PropertyRowContainer* container);

public slots:
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
	bool isContainer() const{ return true; }
	bool onActivate( QPropertyTree* tree, bool force);
	bool onContextMenu(QMenu& item, QPropertyTree* tree);
	void redraw(const PropertyDrawContext& context);
	bool onKeyDown(QPropertyTree* tree, const QKeyEvent* key) override;

	void labelChanged() override;
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return userWidgetSize() == 0 ? false : true; }

	PropertyRow* defaultRow(PropertyTreeModel* model);
	const PropertyRow* defaultRow(const PropertyTreeModel* model) const;
	void serializeValue(yasli::Archive& ar);

	const char* elementTypeName() const{ return elementTypeName_; }
	virtual void setValueAndContext(const yasli::ContainerInterface& value, yasli::Archive& ar) {
		fixedSize_ = value.isFixedSize();
		elementTypeName_ = value.elementType().name();
	}
	const char* typeNameForFilter(QPropertyTree* tree) const override;
	yasli::string valueAsString() const;
	// C-array is an example of fixed size container
	bool isFixedSize() const{ return fixedSize_; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_AFTER_NAME; }
	int widgetSizeMin() const{ return userWidgetSize() >=0 ? userWidgetSize() : 42; }

protected:
	void generateMenu(QMenu& menu, QPropertyTree* tree);

	bool fixedSize_;
	const char* elementTypeName_;
	wchar_t buttonLabel_[8];
};
