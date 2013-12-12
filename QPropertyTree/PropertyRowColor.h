/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "Color.h"
#include "PropertyRow.h"

class PropertyRowColor : public PropertyRow
{
public:
	PropertyRowColor();
	WidgetPlacement widgetPlacement() const override{ return WIDGET_AFTER_PULLED; }
	int widgetSizeMin() const override{ return ROW_DEFAULT_HEIGHT; }
	void redraw(const PropertyDrawContext& context) override;
	void closeNonLeaf(const yasli::Serializer& ser) override;

	bool isLeaf() const override{ return false; }
	bool isStatic() const override{ return false; }
private:
	Color value_;
};

