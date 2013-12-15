/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/StringList.h"
using yasli::StringList;

#include "PropertyRow.h"

class QPropertyTree;
class PropertyRowPointer;
struct CreatePointerMenuHandler : PropertyRowMenuHandler
{
	Q_OBJECT
public:
	QPropertyTree* tree;
	PropertyRowPointer* row;
	int index;
	bool useDefaultValue;
public slots:
	void onMenuCreateByIndex();
};

class QMenu;
struct ClassMenuItemAdder
{
	virtual void addAction(QMenu& menu, const char* text, int index);
	virtual QMenu* addMenu(QMenu& menu, const char* text);
	void generateMenu(QMenu& createItem, const StringList& comboStrings);
};

class PropertyRowPointer : public PropertyRow
{
public:
	PropertyRowPointer();

	bool assignTo(yasli::PointerInterface &ptr);
	void setValueAndContext(const yasli::PointerInterface& ptr, yasli::Archive& ar);

	yasli::TypeID baseType() const{ return baseType_; }
	void setBaseType(const yasli::TypeID& baseType) { baseType_ = baseType; }
	const char* derivedTypeName() const{ return derivedTypeName_.c_str(); }
	yasli::TypeID getDerivedType(yasli::ClassFactoryBase* factory) const;
	void setDerivedType(const yasli::TypeID& typeID, yasli::ClassFactoryBase* factory);
	void setFactory(yasli::ClassFactoryBase* factory) { factory_ = factory; }
	yasli::ClassFactoryBase* factory() const{ return factory_; }
	bool onActivate( QPropertyTree* tree, bool force);
	bool onMouseDown(QPropertyTree* tree, QPoint point, bool& changed);
	bool onContextMenu(QMenu &root, QPropertyTree* tree);
	bool isStatic() const{ return false; }
	bool isPointer() const{ return true; }
	int widgetSizeMin() const;
	yasli::wstring generateLabel() const;
	yasli::string valueAsString() const;
	void redraw(const PropertyDrawContext& context);
	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	void serializeValue(yasli::Archive& ar);
protected:

	yasli::TypeID baseType_;
	yasli::string derivedTypeName_;
	yasli::string derivedLabel_;

	// this member is available for instances deserialized from clipboard:
	yasli::ClassFactoryBase* factory_;
};

