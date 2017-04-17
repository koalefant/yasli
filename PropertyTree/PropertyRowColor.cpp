#include "PropertyTree/PropertyRowColor.h"
#include "PropertyTree/PropertyTreeBase.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "Rect.h"

using property_tree::Color;

PropertyRowColor::PropertyRowColor()
: value_(255, 255, 255, 255)
{
}

int PropertyRowColor::widgetSizeMin(const PropertyTreeBase* tree) const
{
	return tree->_defaultRowHeight();
}

void PropertyRowColor::redraw(IDrawContext& context)
{
	context.drawColor(context.widgetRect.adjusted(1,1,-1,-3), value_);
}

void PropertyRowColor::closeNonLeaf(const yasli::Serializer& ser, yasli::Archive& ar)
{
	property_tree::Color* value = ser.cast<property_tree::Color>();
	if (!value)
		return;
	value_ = *value;
}


REGISTER_PROPERTY_ROW(Color, PropertyRowColor)
DECLARE_SEGMENT(PropertyRowColor)
