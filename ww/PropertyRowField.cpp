#include "StdAfx.h"
#include "PropertyRowField.h"
#include "PropertyDrawContext.h"
#include "gdiplus.h"

namespace ww{

PropertyRowField::PropertyRowField()
{
}

PropertyRowField::PropertyRowField(const char* name, const char* nameAlt, const char* typeName)
: PropertyRow(name, nameAlt, typeName)
{
}

PropertyRowField::PropertyRowField(const char* name, const char* nameAlt, const Serializer& ser)
: PropertyRow(name, nameAlt, ser)
{
}


void PropertyRowField::redraw(const PropertyDrawContext& context)
{
    if(multiValue())
		context.drawEntry(L" ... ", false, true);
    else if(userReadOnly())
		context.drawValueText(pulledSelected(), valueAsWString().c_str());
    else
        context.drawEntry(valueAsWString().c_str(), false, false);
}

}
