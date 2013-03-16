#include "stdafx.h"
#include "PropertyRowObject.h"
#include "PropertyTreeModel.h"

namespace ww {

bool PropertyRowObject::assignTo(Object* obj)
{
	if (object_.type() == obj->type()) {
		*obj = object_;
		return true;
	}
	return false;
}

PropertyRow* PropertyRowObject::clone() const
{
	PropertyRowObject* result = new PropertyRowObject();
	result->setNames(name_, label_, typeName_);
	result->object_ = object_;
	return result;
}

PropertyRowObject::~PropertyRowObject()
{
	//if (model_)
	//	model_->unregisterObjectRow(this);
	object_ = Object();
}

void PropertyRowObject::serialize(Archive& ar)
{
	__super::serialize(ar);
}



}
