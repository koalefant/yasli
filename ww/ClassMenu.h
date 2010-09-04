#pragma once
#include "yasli/StringList.h"

namespace ww{

struct ClassMenuItemAdder
{
	virtual void operator()(PopupMenuItem& root, int index, const char* text){
		root.add(text);
	}

	void generateMenu(PopupMenuItem& createItem, const StringList& comboStrings);
};


}

