#include "PropertyRowColor.h"
#include "PropertyDrawContext.h"
#include "QPropertyTree.h"
#include <QPainter>
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"

PropertyRowColor::PropertyRowColor()
: value_(255, 255, 255, 255)
{
}

void PropertyRowColor::redraw(const PropertyDrawContext& context)
{
	context.painter->save();
	context.painter->setBrush(QBrush(QColor(value_.r, value_.g, value_.b, value_.a)));
	context.painter->setPen(QPen(context.tree->palette().color(QPalette::Shadow)));
	QRect rt = context.widgetRect.adjusted(1,1,-1,-3);
	context.painter->drawRoundedRect(rt, 2, 2);
	context.painter->restore();
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
