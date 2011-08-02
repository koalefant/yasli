/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyTreeOperator.h"
#include "PropertyRow.h"
#include "yasli/EnumDescription.h"
#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"
#include "yasli/PointersImpl.h"

namespace ww{

YASLI_ENUM_BEGIN_NESTED(PropertyTreeOperator, Type, "PropertyTreeOp")
YASLI_ENUM_VALUE_NESTED(PropertyTreeOperator, REPLACE, "Replace")
YASLI_ENUM_VALUE_NESTED(PropertyTreeOperator, ADD, "Add")
YASLI_ENUM_VALUE_NESTED(PropertyTreeOperator, REMOVE, "Remove")
YASLI_ENUM_END()

PropertyTreeOperator::PropertyTreeOperator(const TreePath& path, PropertyRow* row)
: type_(REPLACE)
, path_(path)
, index_(-1)
, row_(row)
{
}

PropertyTreeOperator::PropertyTreeOperator()
: type_(NONE)
, index_(-1)
{
}

PropertyTreeOperator::~PropertyTreeOperator()
{
}

void PropertyTreeOperator::serialize(yasli::Archive& ar)
{
    ar(type_, "type", "Type");
    ar(path_, "path", "Path");
    ar(row_, "row", "Row");
    ar(index_, "index", "Index");
}

}
