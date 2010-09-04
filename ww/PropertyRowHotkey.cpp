#include "StdAfx.h"

#include "ww/PropertyRowImpl.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyTreeModel.h"

#include "ww/HotkeyDialog.h"
#include "ww/PopupMenu.h"
#include "ww/Serialization.h"

#include "yasli/TypesFactory.h"

#include "KeyPress.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window.h"

namespace ww{

class PropertyRowHotkey : public PropertyRowImpl<KeyPress, PropertyRowHotkey>, public sigslot::has_slots{
public:
	PropertyRowHotkey(void* object, int size, const char* name, const char* nameAlt, const char* typeName);
	PropertyRowHotkey() {}
	bool onActivate(PropertyTree* tree, bool force);
	bool onContextMenu(PopupMenuItem& root, PropertyTree* tree);
	void onMenuClear(PropertyTreeModel* model);
	std::string valueAsString() const{ return value().toString(false); }
protected:
};

PropertyRowHotkey::PropertyRowHotkey(void* object, int size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<KeyPress, PropertyRowHotkey>(object, size, name, nameAlt, typeName)
{

}


bool PropertyRowHotkey::onActivate(PropertyTree* tree, bool force)
{
	KeyPress& key = value();

	ww::HotkeyDialog hotkeyDialog(tree, key);
	if(hotkeyDialog.showModal() == RESPONSE_OK){
        tree->model()->push(this);
		key = hotkeyDialog.get();
		tree->model()->rowChanged(this);
		return true;
	}
	else
		return false;
}

bool PropertyRowHotkey::onContextMenu(PopupMenuItem& root, PropertyTree* tree)
{
	if(!root.empty())
		root.addSeparator();
	root.add(TRANSLATE("Clear"), tree->model()).connect(this, &PropertyRowHotkey::onMenuClear);
	return __super::onContextMenu(root, tree);
}

void PropertyRowHotkey::onMenuClear(PropertyTreeModel* model)
{
    model->push(this);
	value() = KeyPress();
	model->rowChanged(this);
}

DECLARE_SEGMENT(PropertyRowHotkey)
REGISTER_PROPERTY_ROW(KeyPress, PropertyRowHotkey);

}
