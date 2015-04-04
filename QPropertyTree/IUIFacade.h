#pragma once

#include "Rect.h"

class IMenu;

class PropertyTree;
class PropertyRow;
class PropertyTreeModel;
class PropertyRowNumberField;
class PropertyRowString;

class InplaceWidget
{
public:
	InplaceWidget(PropertyTree* tree) : tree_(tree) {}
	virtual ~InplaceWidget() {}
	virtual void* actualWidget() { return 0; }
	virtual void showPopup() {}
	virtual void commit() = 0;
	PropertyTree* tree() { return tree_; }
protected:
	PropertyTree* tree_;
};

struct ComboBoxClientRow {
	virtual void populateComboBox(std::vector<std::string>* strings, int* selectedIndex, PropertyTree* tree) = 0;
	virtual bool onComboBoxSelected(const char* text, PropertyTree* tree) = 0;
};

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

	virtual InplaceWidget* createComboBox(ComboBoxClientRow* client) = 0;
	virtual InplaceWidget* createNumberWidget(PropertyRowNumberField* row) = 0;
	virtual InplaceWidget* createStringWidget(PropertyRowString* row) = 0;
};

}
using namespace property_tree;
