#pragma once

#include "Rect.h"

class IMenu;

namespace property_tree {


enum Cursor
{
	CURSOR_SLIDE
};

struct IUIFacade
{
	virtual ~IUIFacade() {}
	virtual IMenu* createMenu() = 0;
	virtual void setCursor(Cursor cursor) = 0;
	virtual void unsetCursor() = 0;
	virtual Point cursorPosition() = 0;
	virtual int textWidth(const char* text, Font font) = 0;
	virtual Point screenSize() = 0;
};

}
using namespace property_tree;
