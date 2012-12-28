/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyRowField.h"
#include "PropertyDrawContext.h"
#include "gdiplusUtils.h"

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
