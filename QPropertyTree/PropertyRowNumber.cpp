/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "Serialization.h"
#include "PropertyRowNumber.h"

#define REGISTER_NUMBER_ROW(TypeName, postfix) \
	typedef PropertyRowNumber<TypeName> PropertyRow##postfix; \
	YASLI_CLASS(PropertyRow, PropertyRow##postfix, #TypeName);

using yasli::string;

REGISTER_NUMBER_ROW(float, Float)
REGISTER_NUMBER_ROW(double , Double)

REGISTER_NUMBER_ROW(char, Char)
REGISTER_NUMBER_ROW(signed char, SignedChar)
REGISTER_NUMBER_ROW(unsigned char, UnsignedChar)

REGISTER_NUMBER_ROW(short, Short)
REGISTER_NUMBER_ROW(int, Int)
REGISTER_NUMBER_ROW(long, Long)
REGISTER_NUMBER_ROW(long long, LongLong)
REGISTER_NUMBER_ROW(unsigned short, UnsignedShort)
REGISTER_NUMBER_ROW(unsigned int, UnsignedInt)
REGISTER_NUMBER_ROW(unsigned long, UnsignedLong)
REGISTER_NUMBER_ROW(unsigned long long, UnsignedLongLong)

#undef REGISTER_NUMBER_ROW

DECLARE_SEGMENT(PropertyRowNumber)

// ---------------------------------------------------------------------------
