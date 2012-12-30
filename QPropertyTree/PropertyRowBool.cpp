/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowBool.h"
#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "yasli/ClassFactory.h"
#include "Serialization.h"

YASLI_CLASS(PropertyRow, PropertyRowBool, "bool");

PropertyRowBool::PropertyRowBool(const char* name, const char* label, bool value)
: PropertyRow(name, label, "bool")
, value_(value)
{
}

bool PropertyRowBool::assignTo(void* object, size_t size)
{
	YASLI_ASSERT(size == sizeof(bool));
	*reinterpret_cast<bool*>(object) = value_;
	return true;
}

void PropertyRowBool::redraw(const PropertyDrawContext& context)
{
	context.drawCheck(widgetRect(), userReadOnly(), multiValue() ? CHECK_IN_BETWEEN : (value_ ? CHECK_SET : CHECK_NOT_SET));
}

bool PropertyRowBool::onActivate(QPropertyTree* tree, bool force)
{
	if (!this->userReadOnly()) {
		tree->model()->push(this);
		value_ = !value_;
		tree->model()->rowChanged(this);
		return true;
	}
	else
		return false;
}

void PropertyRowBool::digestReset(const QPropertyTree* tree)
{
	digest_ = value_ ? toWideChar(labelUndecorated()) : L"";
}

void PropertyRowBool::serializeValue(yasli::Archive& ar)
{
    ar(value_, "value", "Value");
}
