/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include <math.h>

#include "Factory.h"
#include "PropertyRowStringListValue.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "QPropertyTree.h"
#include "ConstStringList.h"

#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include <QMenu>
#include <QComboBox>

using yasli::StringList;
using yasli::StringListValue;

REGISTER_PROPERTY_ROW(StringListValue, PropertyRowStringListValue)


PropertyRowWidget* PropertyRowStringListValue::createWidget(QPropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree);
}

// ---------------------------------------------------------------------------
REGISTER_PROPERTY_ROW(StringListStaticValue, PropertyRowStringListStaticValue)

PropertyRowWidget* PropertyRowStringListStaticValue::createWidget(QPropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree);
}

DECLARE_SEGMENT(PropertyRowStringList)

// vim:ts=4 sw=4:
