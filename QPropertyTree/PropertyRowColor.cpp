#include "PropertyRowColor.h"
#include "PropertyDrawContext.h"
#include "PropertyTree.h"
#include <QPainter>
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "Rect.h"

PropertyRowColor::PropertyRowColor()
: value_(255, 255, 255, 255)
{
}

int PropertyRowColor::widgetSizeMin(const PropertyTree* tree) const
{
	return tree->_defaultRowHeight();
}

void PropertyRowColor::redraw(PropertyDrawContext& context)
{
	context.drawColor(context.widgetRect.adjusted(1,1,-1,-3), value_);
}

void PropertyRowColor::closeNonLeaf(const yasli::Serializer& ser)
{
	Color* value = ser.cast<Color>();
	if (!value)
		return;
	value_ = *value;
}


REGISTER_PROPERTY_ROW(Color, PropertyRowColor)
DECLARE_SEGMENT(PropertyRowColor)
