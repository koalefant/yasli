#pragma once
#include "PropertyRow.h"
#include "yasli/MemoryWriter.h"
#include "yasli/Pointers.h"
#include "yasli/Object.h"

namespace yasli {
	class Archive;
	class Serializer;
	class MemoryWriter;
};

namespace ww {

using yasli::Serializer;
using yasli::Object;
using yasli::Archive;
using yasli::MemoryWriter;

class PropertyRowObject : public PropertyRow {
public:
	enum { Custom = false };
	PropertyRowObject(const char* name, const char* label, const Object& obj, PropertyTreeModel* model);
	~PropertyRowObject();
	bool isObject() const{ return true; }
	PropertyRow* clone() const;
	bool assignTo(Object* obj);
	void serialize(Archive& ar);
	const Object& object() const{ return object_; }
	void setModel(PropertyTreeModel* model) { model_ = model; }
protected:
	Object object_;
	PropertyTreeModel* model_;
};

}
