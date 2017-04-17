#pragma once
#include <QObject>
#include "PropertyTree/PropertyRow.h"

class PropertyRowFileOpen;
class PropertyTreeBase;

struct FileOpenMenuHandler : PropertyRowMenuHandler
{
	PropertyRowFileOpen* self;
	PropertyTreeBase* tree;

	FileOpenMenuHandler(PropertyRowFileOpen* self, PropertyTreeBase* tree)
	: self(self)
	, tree(tree)
	{
	}

	void onMenuActivate();
	void onMenuClear();
};

yasli::string extractExtensionFromFilter(const char* fileSelectorFilter);
