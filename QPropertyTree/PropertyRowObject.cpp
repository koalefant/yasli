#include "PropertyRowObject.h"
#include "PropertyTreeModel.h"

PropertyRowObject::PropertyRowObject(const char* name, const char* label, const Object& object, PropertyTreeModel* model)
: PropertyRow(name, label, object.type().name())
, object_(object)
, model_(model)
{
	if (object.address())
		model_->registerObjectRow(this);
}

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
	return new PropertyRowObject(name_, label_, Object(0, object_.type(), 0, 0, 0), model_);
}

PropertyRowObject::~PropertyRowObject()
{
	if (model_)
		model_->unregisterObjectRow(this);
	object_ = Object();
}

void PropertyRowObject::serialize(Archive& ar)
{
    PropertyRow::serialize(ar);
}

