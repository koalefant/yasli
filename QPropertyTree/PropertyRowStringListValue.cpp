/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
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
//#include "ClassMenu.h"

#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include <QtGui/QMenu>
#include <QtGui/QComboBox>

using yasli::StringList;
using yasli::StringListValue;

REGISTER_PROPERTY_ROW(StringListValue, PropertyRowStringListValue)

PropertyRowStringListValue::PropertyRowStringListValue(const char* name, const char* label, const StringListValue& value)
: PropertyRowImpl<StringListValue, PropertyRowStringListValue>(name, label, value)
{
}

PropertyRowStringListValue::PropertyRowStringListValue(void* object, size_t size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<StringListValue, PropertyRowStringListValue>(object, size, name, label, typeName)
{
}

PropertyRowWidget* PropertyRowStringListValue::createWidget(QPropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree);
}

// ---------------------------------------------------------------------------
REGISTER_PROPERTY_ROW(StringListStaticValue, PropertyRowStringListStaticValue)

PropertyRowStringListStaticValue::PropertyRowStringListStaticValue(const char* name, const char* label, const StringListStaticValue& value)
: PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>(name, label, value)
{
}

PropertyRowStringListStaticValue::PropertyRowStringListStaticValue(void* object, size_t size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>(object, size, name, label, typeName)
{
}

PropertyRowWidget* PropertyRowStringListStaticValue::createWidget(QPropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree);
}

// vim:ts=4 sw=4: