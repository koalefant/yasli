/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
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

PropertyRowBool::PropertyRowBool()
: value_(false)
{
}

bool PropertyRowBool::assignToPrimitive(void* object, size_t size) const
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
		tree->model()->rowAboutToBeChanged(this);
		value_ = !value_;
		tree->model()->rowChanged(this);
		return true;
	}
	else
		return false;
}

void PropertyRowBool::serializeValue(yasli::Archive& ar)
{
    ar(value_, "value", "Value");
}
