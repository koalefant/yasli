#include "StdAfx.h"
#include "PropertyRowField.h"
#include "PropertyTreeDrawing.h"
#include "gdiplus.h"

namespace ww{

PropertyRowField::PropertyRowField()
{
	widgetSizeMin_ = 90;
}

PropertyRowField::PropertyRowField(const char* name, const char* nameAlt, const char* typeName)
: PropertyRow(name, nameAlt, typeName)
{
	widgetSizeMin_ = 90;
}

PropertyRowField::PropertyRowField(const char* name, const char* nameAlt, const Serializer& ser)
: PropertyRow(name, nameAlt, ser)
{
	widgetSizeMin_ = 90;
}


void PropertyRowField::redraw(Gdiplus::Graphics* gr, const Gdiplus::Rect& widgetRect, const Gdiplus::Rect& floorRect)
{
    if(multiValue())
        drawEdit(gr, widgetRect, L" ... ", propertyTreeDefaultFont(), false, true);
    else if(readOnly())
        drawStaticText(gr, widgetRect);
    else
        drawEdit(gr, widgetRect, valueAsWString().c_str(), propertyTreeDefaultFont(), false, false);
}

}
