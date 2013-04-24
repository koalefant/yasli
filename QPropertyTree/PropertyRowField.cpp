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
#include <QtGui/QIcon>

void PropertyRowField::redraw(const PropertyDrawContext& context)
{
	int buttonCount = this->buttonCount();
	int offset = 0;
	for (int i = 0; i < buttonCount; ++i) {
		const QIcon& icon = buttonIcon(context.tree, i);
		int width = 16;
		QRect iconRect(context.widgetRect.right() - offset - width, context.widgetRect.top(), width, context.widgetRect.height());
		icon.paint(context.painter, iconRect);
		offset += width;
	}

	int iconSpace = offset ? offset + 2 : 0;
    if(multiValue())
		context.drawEntry(L" ... ", false, true, iconSpace);
    else if(userReadOnly())
		context.drawValueText(pulledSelected(), valueAsWString().c_str());
    else
        context.drawEntry(valueAsWString().c_str(), usePathEllipsis(), false, iconSpace);

}

