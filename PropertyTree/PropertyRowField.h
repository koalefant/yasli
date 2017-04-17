/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include <PropertyTree/PropertyRow.h>
#include <PropertyTree/IDrawContext.h>

class PropertyRowField : public PropertyRow
{
public:
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	int widgetSizeMin(const PropertyTreeBase* tree) const override;

	virtual int buttonCount() const{ return 0; }
	virtual property_tree::Icon buttonIcon(const PropertyTreeBase* tree, int index) const;
	virtual bool usePathEllipsis() const { return false; }
	virtual bool onActivateButton(int buttonIndex, const PropertyActivationEvent& e) { return false; }
	int hitButton(const PropertyTreeBase* tree, const Point& p) const;

	void redraw(IDrawContext& context) override;
	bool onActivate(const PropertyActivationEvent& e) override;
protected:
	Rect fieldRect(const PropertyTreeBase* tree) const;
	void drawButtons(int* offset);

	mutable RowWidthCache widthCache_;
};

