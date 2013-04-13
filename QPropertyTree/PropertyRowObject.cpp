/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


#include "PropertyRowObject.h"
#include "PropertyTreeModel.h"

PropertyRowObject::PropertyRowObject()
: model_(0)
{
}

bool PropertyRowObject::assignTo(Object* obj)
{
	if (object_.type() == obj->type()) {
		*obj = object_;
		return true;
	}
	return false;
}

PropertyRow* PropertyRowObject::cloneSelf() const
{
	PropertyRowObject* result =  new PropertyRowObject();
	result->setNames(name_, label_, typeName_);
	result->setModel(model_);
	return result;
}

PropertyRowObject::~PropertyRowObject()
{
	object_ = Object();
}

void PropertyRowObject::serialize(Archive& ar)
{
    PropertyRow::serialize(ar);
}

