#pragma once 
#include "ww/PropertyTree.h"
#include "PropertyTree/IMenu.h"
#include "PropertyTree/IUIFacade.h"

namespace property_tree {

struct wwMenuAction : IMenuAction
{
};

struct wwMenu : IMenu
{
	bool isEmpty() override { return false; }
	IMenu* addMenu(const char* text) override { return new wwMenu(); }
	IMenu* findMenu(const char* text) { return 0; }
	void addSeparator() { }
	IMenuAction* addAction(const char* text, int flags = 0) { return new wwMenuAction(); }
	void exec(const Point& point) {}
};

class wwUIFacade : public IUIFacade
{
public:
	wwUIFacade(ww::PropertyTree* tree) : tree_(tree) {}

	HWND hwnd() override;
	
	IMenu* createMenu() override;
	void setCursor(Cursor cursor) override;
	void unsetCursor() override;
	Point cursorPosition() override;
	int textWidth(const char* text, Font font) override;
	Point screenSize() override;

	InplaceWidget* createComboBox(ComboBoxClientRow* client) override;
	InplaceWidget* createNumberWidget(PropertyRowNumberField* row) override;
	InplaceWidget* createStringWidget(PropertyRowString* row) override;
private:
	ww::PropertyTree* tree_;
};

}
