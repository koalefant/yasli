/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "PropertyRow.h"

namespace ww{

class PropertyRowField : public PropertyRow
{
public:
    PropertyRowField();
    PropertyRowField(const char* name, const char* nameAlt, const char* typeName);
    PropertyRowField(const char* name, const char* nameAlt, const Serializer& ser);

	WidgetPlacement widgetPlacement() const{ return WIDGET_VALUE; }
	int widgetSizeMin() const{ 
		if (userWidgetSize() >= 0)
			return userWidgetSize();
		else
			return 40;
	}

    void redraw(const PropertyDrawContext& context);
};

}
