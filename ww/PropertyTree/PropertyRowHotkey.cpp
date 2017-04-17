/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/PropertyTree.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/IMenu.h"

#include "ww/HotkeyDialog.h"
#include "ww/PopupMenu.h"
#include "ww/Serialization.h"

#include "yasli/ClassFactory.h"

#include "KeyPress.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window32.h"

namespace ww{

class PropertyRowHotkey : public PropertyRowImpl<KeyPress>{
public:
	bool onActivate(const PropertyActivationEvent & e) override;
	bool onContextMenu(property_tree::IMenu& root, ::PropertyTreeBase* tree) override;
	void onMenuClear(PropertyTreeModel* model);
	std::string valueAsString() const override{ return value().toString(false); }
protected:
};

bool PropertyRowHotkey::onActivate(const PropertyActivationEvent& e)
{
	KeyPress& key = value();

	ww::PropertyTree* wwTree = safe_cast<ww::PropertyTree*>(e.tree);
	ww::HotkeyDialog hotkeyDialog(wwTree, key);
	if(hotkeyDialog.showModal() == RESPONSE_OK){
        e.tree->model()->rowAboutToBeChanged(this);
		key = hotkeyDialog.get();
		e.tree->model()->rowChanged(this);
		return true;
	}
	else
		return false;
}

struct HotkeyMenuHandler : PropertyRowMenuHandler
{
	PropertyRowHotkey* row;
	PropertyTreeBase* tree;
	HotkeyMenuHandler(PropertyRowHotkey* row, PropertyTreeBase* tree) : tree(tree), row(row) {}

	void onMenuClear()
	{
		row->onMenuClear(tree->model());
	}
};

bool PropertyRowHotkey::onContextMenu(property_tree::IMenu& root, PropertyTreeBase* tree)
{
	if(!root.isEmpty())
		root.addSeparator();
	HotkeyMenuHandler* handler = new HotkeyMenuHandler(this, tree);
	root.addAction(TRANSLATE("Clear"), 0, handler, &HotkeyMenuHandler::onMenuClear);
	return __super::onContextMenu(root, tree);
}

void PropertyRowHotkey::onMenuClear(PropertyTreeModel* model)
{
    model->rowAboutToBeChanged(this);
	value() = KeyPress();
	model->rowChanged(this);
}

DECLARE_SEGMENT(PropertyRowHotkey)
REGISTER_PROPERTY_ROW(KeyPress, PropertyRowHotkey);

}
