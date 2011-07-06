#include "StdAfx.h"
#include "PropertyRowField.h"
#include "PropertyDrawContext.h"
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


void PropertyRowField::redraw(const PropertyDrawContext& context)
{
    if(multiValue())
		context.drawEntry(L" ... ", false, true);
    else if(readOnly())
		context.drawValueText(pulledSelected(), valueAsWString().c_str());
    else
        context.drawEntry(valueAsWString().c_str(), false, false);
}

}
