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
		if (userWidgetSize() != 0)
			return userWidgetSize();
		else
			return 40;
	}

    void redraw(const PropertyDrawContext& context);
};

}
