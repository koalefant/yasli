/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowField.h"
#include "PropertyDrawContext.h"

void PropertyRowField::redraw(const PropertyDrawContext& context)
{
    if(multiValue())
		context.drawEntry(L" ... ", false, true);
    else if(userReadOnly())
		context.drawValueText(pulledSelected(), valueAsWString().c_str());
    else
        context.drawEntry(valueAsWString().c_str(), false, false);
}

