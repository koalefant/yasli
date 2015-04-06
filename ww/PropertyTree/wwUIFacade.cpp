#include "wwUIFacade.h"
#include "ww/Win32/Window32.h"

#include <Windows.h>

namespace property_tree {

HWND wwUIFacade::hwnd()
{
	return tree_->_window()->handle();
}


IMenu* wwUIFacade::createMenu()
{
	return new wwMenu();
}

void wwUIFacade::setCursor(Cursor cursor)
{
}

void wwUIFacade::unsetCursor()
{
}

Point wwUIFacade::cursorPosition()
{
	POINT pt = { 0, 0 };
	GetCursorPos(&pt);
	return Point(pt.x, pt.y);
}

int wwUIFacade::textWidth(const char* text, Font font)
{
	return 20;
}

Point wwUIFacade::screenSize()
{
	return Point(1920, 1080);
}


InplaceWidget* wwUIFacade::createComboBox(ComboBoxClientRow* client)
{
	return 0;
}

InplaceWidget* wwUIFacade::createNumberWidget(PropertyRowNumberField* row)
{
	return 0;
}

InplaceWidget* wwUIFacade::createStringWidget(PropertyRowString* row)
{
	return 0;
}

}
