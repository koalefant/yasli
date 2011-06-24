#include "StdAfx.h"
#include "ww/Serialization.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyTreeDrawing.h"
#include "ww/PropertyRowImpl.h"
#include "ww/FileDialog.h"
#include "ww/PopupMenu.h"
#include "ww/Files.h"
#include "yasli/TypesFactory.h"
#include "ww/FileSelector.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window.h"
#include "ww/Unicode.h"
#include "gdiplus.h"

using std::string;

namespace ww{

class PropertyRowFileSelector : public PropertyRowImpl<FileSelector, PropertyRowFileSelector>, public sigslot::has_slots{
	bool locked_;
public:
	PropertyRowFileSelector(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	PropertyRowFileSelector() {}
	bool activateOnAdd() const{ return true; }
	bool onActivate(PropertyTree* tree, bool force);
	bool onContextMenu(PopupMenuItem& root, PropertyTree* tree);
	void onMenuClear(PropertyTreeModel* model);

	std::string valueAsString() const{ return value().c_str(); }

	void serializeValue(yasli::Archive& ar){
		string fileName = value_.c_str();
		ar.serialize(fileName, "value", "Value");
		if(ar.isInput())
			value_ = fileName.c_str();
	}
};


PropertyRowFileSelector::PropertyRowFileSelector(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowImpl<FileSelector, PropertyRowFileSelector>(object, size, name, nameAlt, typeName), locked_(false)
{
}

bool PropertyRowFileSelector::onActivate(PropertyTree* tree, bool force)
{
	if(locked_)
		return false;
	locked_ = true;

	FileSelector& selector = value();
	const FileSelector::Options* options = selector.options();
	string title = std::string("(") + options->filter + ")";
	const char* masks[] = { title.c_str(), selector.options()->filter.c_str(), 0 };

	string root = Files::fixSlashes(options->rootDirectory.c_str());
	string fileName = selector.c_str();
    if(root == ".")
        root = Files::currentDirectory();
	if(!root.empty() && fileName.find('\\') == string::npos)
    {
        if(!fileName.empty())
		    fileName = root[root.size() - 1] == '\\' ? root + fileName : root + '\\' + fileName;
    }

	ww::FileDialog fileDialog(tree, options->save, masks, root.c_str(), fileName.c_str());
	if(fileDialog.showModal()){
        tree->model()->push(this);
		if(!root.empty()){
			string relativePath = Files::relativePath(fileDialog.fileName(), root.c_str());;
			if(!relativePath.empty())
				selector = relativePath.c_str();
			else
				selector = fileDialog.fileName();
		}
		else
			selector = fileDialog.fileName();
		tree->model()->rowChanged(this);
		locked_ = false;
		return true;
	}
	else{
		locked_ = false;
		return false;
	}
}

bool PropertyRowFileSelector::onContextMenu(PopupMenuItem& root, PropertyTree* tree)
{
	if(!root.empty())
		root.addSeparator();
	root.add(TRANSLATE("Clear"), tree->model()).connect(this, &PropertyRowFileSelector::onMenuClear);
	return __super::onContextMenu(root, tree);
}

void PropertyRowFileSelector::onMenuClear(PropertyTreeModel* model)
{
    model->push(this);
	value() = "";
	model->rowChanged(this);
}

DECLARE_SEGMENT(PropertyRowFileSelector)
REGISTER_PROPERTY_ROW(FileSelector, PropertyRowFileSelector);

}
