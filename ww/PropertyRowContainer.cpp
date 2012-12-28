/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <math.h>
#include <memory>

#include "ww/PropertyRowContainer.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTree.h"

#include "ww/PropertyRowPointer.h"
#include "ww/PropertyRowNumeric.h"
#include "ww/ConstStringList.h"
#include "ww/PopupMenu.h"
#include "ww/ComboBox.h"
#include "ww/CheckComboBox.h"
#include "ww/Entry.h"
#include "ClassMenu.h"

#include "yasli/BitVector.h"
#include "yasli/Archive.h"
#include "yasli/BitVectorImpl.h"
#include "yasli/ClassFactory.h"
#include "yasli/Enum.h"
#include "ww/SafeCast.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "ww/Color.h"
#include "gdiplusUtils.h"

#ifndef TRANSLATE
# define TRANSLATE(x) x
#endif

namespace ww{

YASLI_CLASS(PropertyRow, PropertyRowContainer, "Container");

PropertyRowContainer::PropertyRowContainer(const char* name = "", const char* label = "", const char* typeName = "", const char* elementTypeName = "", bool readOnly = false)
: PropertyRow(name, label, typeName)
, fixedSize_(readOnly)
, elementTypeName_(elementTypeName)
{
	buttonLabel_[0] = 0;

	if(pulledUp())
		_setExpanded(true);		
}

struct ClassMenuItemAdderRowContainer : ClassMenuItemAdder{
	ClassMenuItemAdderRowContainer(PropertyRowContainer* row, PropertyTree* tree, bool insert = false) 
	: row_(row)
	, tree_(tree)
	, insert_(insert) {}    

	void operator()(PopupMenuItem& root, int index, const char* text){
		if(!insert_)
			root.add(text, index, tree_).connect(row_, &PropertyRowContainer::onMenuAppendPointerByIndex);
	}
protected:
	PropertyRowContainer* row_;
	PropertyTree* tree_;
	bool insert_;
};

void PropertyRowContainer::redraw(const PropertyDrawContext& context)
{
	using namespace Gdiplus;
	using Gdiplus::Color;
	using Gdiplus::Rect;
	Rect widgetRect = gdiplusRect(context.widgetRect);
	if (widgetRect.Width == 0)
		return;
	Rect rt(widgetRect.X, widgetRect.Y + 1, widgetRect.Width - 2, widgetRect.Height - 2);
	Color brushColor = gdiplusSysColor(COLOR_BTNFACE);
	LinearGradientBrush brush(Rect(rt.X, rt.Y, rt.Width, rt.Height + 3), Color(255, 0, 0, 0), brushColor, LinearGradientModeVertical);

	Color colors[3] = { brushColor, brushColor, gdiplusSysColor(COLOR_3DSHADOW) };
	Gdiplus::REAL positions[3] = { 0.0f, 0.6f, 1.0f };
	brush.SetInterpolationColors(colors, positions, 3);

	fillRoundRectangle(context.graphics, &brush, rt, gdiplusSysColor(COLOR_3DSHADOW), 6);

	Color textColor;
    textColor.SetFromCOLORREF(userReadOnly() ? GetSysColor(COLOR_3DSHADOW) : GetSysColor(COLOR_WINDOWTEXT));
	Gdiplus::SolidBrush textBrush(textColor);
	RectF textRect( float(widgetRect.X), float(widgetRect.Y), float(widgetRect.Width), float(widgetRect.Height) );
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(StringTrimmingNone);
	wchar_t* text = multiValue() ? L"..." : buttonLabel_; 
	context.graphics->DrawString(text, (int)wcslen(text), propertyTreeDefaultFont(), textRect, &format, &textBrush);
}

bool PropertyRowContainer::onKeyDown(PropertyTree* tree, KeyPress key)
{
	if(key == KeyPress(KEY_DELETE, KEY_MOD_SHIFT)){
		onMenuClear(tree->model());
		return true;
	}
	if(key == KeyPress(KEY_INSERT)){
		onMenuAppendElement(tree);
		return true;
	}
	return __super::onKeyDown(tree, key);
}

bool PropertyRowContainer::onActivate( PropertyTree* tree, bool force)
{
    if(userReadOnly())
        return false;
    PopupMenu menu;
    generateMenu(menu.root(), tree);
    menu.spawn(tree->_toScreen(Vect2(widgetPos_, pos_.y + ROW_DEFAULT_HEIGHT)), tree);
    return true;
}

void PropertyRowContainer::generateMenu(PopupMenuItem& root, PropertyTree* tree)
{
	if (fixedSize_)
	{
		root.add("[ Fixed Size Container ]").enable(false);
	}
    else if(userReadOnly())
    {
		root.add("[ Read Only Container ]").enable(false);
	}
	else
	{
		PopupMenuItem& createItem = root.add(TRANSLATE("Add"), tree)
			.connect(this, &PropertyRowContainer::onMenuAppendElement)
			.setHotkey(KeyPress(VK_INSERT));

		root.addSeparator();

		PropertyRow* row = defaultRow(tree->model());
		if(row && row->isPointer()){
			PropertyRowPointer* pointerRow = safe_cast<PropertyRowPointer*>(row);
			ClassMenuItemAdderRowContainer(this, tree).generateMenu(createItem, tree->model()->typeStringList(pointerRow->baseType()));
		}
		createItem.enable(true);

	    root.add(TRANSLATE("Remove All"), tree->model()).connect(this, &PropertyRowContainer::onMenuClear)
		    .setHotkey(KeyPress(KEY_DELETE, KEY_MOD_SHIFT))
		    .enable(!userReadOnly());
	}

}

bool PropertyRowContainer::onContextMenu(PopupMenuItem& root, PropertyTree* tree)
{
	if(!root.empty())
		root.addSeparator();

    generateMenu(root, tree);

	if(pulledUp())
		return !root.empty();

	return PropertyRow::onContextMenu(root, tree);
}


void PropertyRowContainer::onMenuClear(PropertyTreeModel* model)
{
    model->push(this);
	clear();
	model->rowChanged(this);
}

PropertyRow* PropertyRowContainer::defaultRow(PropertyTreeModel* model)
{
	PropertyRow* defaultType = model->defaultType(elementTypeName_);
	//YASLI_ASSERT(defaultType);
	//YASLI_ASSERT(defaultType->numRef() == 1);
	return defaultType;
}

const PropertyRow* PropertyRowContainer::defaultRow(const PropertyTreeModel* model) const
{
	const PropertyRow* defaultType = model->defaultType(elementTypeName_);
	return defaultType;
}


void PropertyRowContainer::onMenuAppendElement(PropertyTree* tree)
{
	tree->model()->push(this);
	PropertyRow* defaultType = defaultRow(tree->model());
	YASLI_ESCAPE(defaultType != 0, return);
	SharedPtr<PropertyRow> clonedRow = defaultType->clone();
	// clonedRow->setFullRow(true); TODO
	if(count() == 0)
		tree->expandRow(this);
	add(clonedRow);
	setMultiValue(false);
	if(expanded())
		tree->model()->selectRow(clonedRow, true);
	tree->expandRow(clonedRow);
	PropertyTreeModel::Selection sel = tree->model()->selection();
	tree->model()->rowChanged(this);
	tree->model()->setSelection(sel);
	tree->update(); 
	clonedRow = tree->selectedRow();
	if(clonedRow->activateOnAdd())
		clonedRow->onActivate(tree, false);
}

void PropertyRowContainer::onMenuAppendPointerByIndex(int index, PropertyTree* tree)
{
	PropertyRow* defaultType = defaultRow(tree->model());
	PropertyRow* clonedRow = defaultType->clone();
	// clonedRow->setFullRow(true); TODO
    if(count() == 0)
        tree->expandRow(this);
    add(clonedRow);
	setMultiValue(false);
	PropertyRowPointer* pointer = safe_cast<PropertyRowPointer*>(clonedRow);
	if(expanded())
		tree->model()->selectRow(clonedRow, true);
	tree->expandRow(pointer);
    PropertyTreeModel::Selection sel = tree->model()->selection();
	pointer->onMenuCreateByIndex(index, true, tree);
    tree->model()->setSelection(sel);
	tree->update(); 
}

void PropertyRowContainer::onMenuChildInsertBefore(PropertyRow* child, PropertyTree* tree)
{
    tree->model()->push(this);
	PropertyRow* defaultType = tree->model()->defaultType(elementTypeName_);
	if(!defaultType)
		return;
	PropertyRow* clonedRow = defaultType->clone();
	// clonedRow->setFullRow(true); TODO
	addBefore(clonedRow, child);
	setMultiValue(false);
	tree->model()->selectRow(clonedRow, true);
	PropertyTreeModel::Selection sel = tree->model()->selection();
	tree->model()->rowChanged(clonedRow);
	tree->model()->setSelection(sel);
	tree->update(); 
	clonedRow = tree->selectedRow();
	if(clonedRow->activateOnAdd())
		clonedRow->onActivate(tree, false);
}

void PropertyRowContainer::onMenuChildRemove(PropertyRow* child, PropertyTreeModel* model)
{
    model->push(this);
	erase(child);
	setMultiValue(false);
	model->rowChanged(this);
}

void PropertyRowContainer::digestReset(const PropertyTree* tree)
{
	swprintf_s(buttonLabel_, ARRAY_LEN(buttonLabel_), L"%i", count());

	/*
	wchar_t buffer[14];
	if(multiValue())
		swprintf_s(buffer, L"[...]");
	else
		swprintf_s(buffer, L"[ %i ]", int(count()));*/
	__super::digestReset(tree);
	//digestAppend(buffer);
}

void PropertyRowContainer::serializeValue(Archive& ar)
{
	ar(ConstStringWrapper(constStrings_, elementTypeName_), "elementTypeName", "ElementTypeName");
	ar(fixedSize_, "fixedSize", "fixedSize");
}

string PropertyRowContainer::valueAsString() const
{
    return numericAsString(children_.size());
}

}
// vim:ts=4 sw=4:
