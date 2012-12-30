/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/PointersImpl.h"
#include "yasli/STLImpl.h"

using yasli::Archive;
using yasli::Serializer;
using yasli::Serializers;
using yasli::TypeID;
using yasli::SharedPtr;

enum{
	SERIALIZE_DESIGN = 1 << 0,
	SERIALIZE_STATE  = 1 << 1
};
	

