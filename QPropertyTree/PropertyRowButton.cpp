/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowImpl.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "yasli/decorators/Button.h"
#include "PropertyTree.h"
#include "PropertyDrawContext.h"
#include "PropertyTreeModel.h"
#include "Unicode.h"

using std::string;
using std::wstring;
using yasli::Button;
class PropertyRowButton : public PropertyRowImpl<Button>{
public:
	PropertyRowButton();
	void redraw(PropertyDrawContext& context) override;
	bool onMouseDown(PropertyTree* tree, Point point, bool& changed);
	void onMouseMove(PropertyTree* tree, Point point);
	void onMouseUp(PropertyTree* tree, Point point);
	bool onActivate(PropertyTree* tree, bool force);
	int floorHeight() const{ return 3; }
	string valueAsString() const { return value_ ? value_.text : ""; }
	int widgetSizeMin() const{ 
		if (userWidgetSize() >= 0)
			return userWidgetSize();
		else
			return 60; 
	}
protected:
	bool underMouse_;
};

PropertyRowButton::PropertyRowButton()
: underMouse_(false)
{
}
	

void PropertyRowButton::redraw(PropertyDrawContext& context)
{
	Rect buttonRect(context.widgetRect.adjusted(-1, 0, 1, 2));

	wstring text(toWideChar(value().text ? value().text : labelUndecorated()));
	bool pressed = underMouse_ && value();
	context.drawButton(buttonRect, text.c_str(), pressed, 
					   selected() && context.tree->hasFocusOrInplaceHasFocus(), true, true, false, rowFont(context.tree));
}

bool PropertyRowButton::onMouseDown(PropertyTree* tree, Point point, bool& changed)
{
	if(widgetRect(tree).contains(point)){
		value().pressed = !value().pressed;
		underMouse_ = true;
		tree->repaint();
		return true;
	}
	return false;
}

void PropertyRowButton::onMouseMove(PropertyTree* tree, Point point)
{
	bool underMouse = widgetRect(tree).contains(point);
	if(underMouse != underMouse_){
		underMouse_ = underMouse;
		tree->repaint();
	}
}

void PropertyRowButton::onMouseUp(PropertyTree* tree, Point point)
{
	if(widgetRect(tree).contains(point)){
		onActivate(tree, false);
    }
	else{
        tree->model()->rowAboutToBeChanged(this);
		value().pressed = false;
		tree->repaint();
	}
}

bool PropertyRowButton::onActivate(PropertyTree* tree, bool force)
{
	value().pressed = true;
	tree->model()->rowChanged(this); // Row is recreated here, so don't unlock
	return true;
}

DECLARE_SEGMENT(PropertyRowButton)
REGISTER_PROPERTY_ROW(Button, PropertyRowButton);
