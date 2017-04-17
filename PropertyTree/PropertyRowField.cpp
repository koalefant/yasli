/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowField.h"
#include "IDrawContext.h"
#include "Rect.h"

enum { BUTTON_SIZE = 16 };

Rect PropertyRowField::fieldRect(const PropertyTreeBase* tree) const
{
		Rect fieldRect = widgetRect(tree);
		fieldRect.w -= buttonCount() * BUTTON_SIZE;
		return fieldRect;
}

bool PropertyRowField::onActivate(const PropertyActivationEvent& e)
{
	int buttonCount = this->buttonCount();
	Rect buttonsRect = widgetRect(e.tree);
	buttonsRect.x = buttonsRect.x + buttonsRect.w - buttonCount * BUTTON_SIZE;

	if (e.reason == e.REASON_PRESS)	{

		int buttonIndex = hitButton(e.tree, e.clickPoint);
		if (buttonIndex != -1)
			if (onActivateButton(buttonIndex, e))
				return true;
	}

	if (e.reason == e.REASON_DOUBLECLICK && buttonsRect.contains(e.clickPoint))
		return false;

	return PropertyRow::onActivate(e);
}

void PropertyRowField::redraw(IDrawContext& context)
{
	int buttonCount = this->buttonCount();
	int offset = 0;
	for (int i = 0; i < buttonCount; ++i) {
		Icon icon = buttonIcon(context.tree, i);
		Rect iconRect(context.widgetRect.right() - BUTTON_SIZE * buttonCount + offset, context.widgetRect.top(), BUTTON_SIZE, context.widgetRect.height());
		context.drawIcon(iconRect, icon, userReadOnly() ? ICON_DISABLED : ICON_NORMAL);
		offset += BUTTON_SIZE;
	}

	int iconSpace = offset ? offset + 2 : 0;
    if(multiValue())
		context.drawEntry(context.widgetRect, " ... ", false, userReadOnly() || userRenamable(), iconSpace);
    else if(userReadOnly() || userRenamable())
		context.drawValueText(inlinedSelected(), valueAsString().c_str());
    else
        context.drawEntry(context.widgetRect, valueAsString().c_str(), usePathEllipsis(), false, iconSpace);

}

Icon PropertyRowField::buttonIcon(const PropertyTreeBase* tree, int index) const
{
	return Icon();
}

int PropertyRowField::widgetSizeMin(const PropertyTreeBase* tree) const
{ 
	if (userWidgetSize() >= 0)
		return userWidgetSize();

	if (userWidgetToContent_)
		return widthCache_.getOrUpdate(tree, this, 0);
	else
		return 40;
}

int PropertyRowField::hitButton(const PropertyTreeBase* tree, const Point& point) const
{
	int buttonCount = this->buttonCount();
	Rect buttonsRect = widgetRect(tree);
	buttonsRect.x = buttonsRect.x + buttonsRect.w - buttonCount * BUTTON_SIZE;

	if (buttonsRect.contains(point)) {
		int buttonIndex = (point.x() - buttonsRect.x) / BUTTON_SIZE;
		if (buttonIndex >= 0 && buttonIndex < buttonCount)
			return buttonIndex;
	}
	return -1;
}
