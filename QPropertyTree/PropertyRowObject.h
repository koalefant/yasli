/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

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

using yasli::Serializer;
using yasli::Object;
using yasli::Archive;
using yasli::MemoryWriter;

class PropertyRowObject : public PropertyRow {
public:
	enum { Custom = false };
	PropertyRowObject();
	~PropertyRowObject();
	void setValue(const Object& obj) { object_ = obj; }
	void setModel(PropertyTreeModel* model) { model_ = model; }
	bool isObject() const{ return true; }
	PropertyRow* cloneSelf() const;
	bool assignTo(Object* obj);
	void serialize(Archive& ar);
	const Object& object() const{ return object_; }
protected:
	Object object_;
	PropertyTreeModel* model_;
};

