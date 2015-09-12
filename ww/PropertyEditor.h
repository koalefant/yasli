/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/API.h"
#include "yasli/Serializer.h"
#include "Win32/Types.h"

namespace ww{
class Widget;

enum EditFlags{
	COMPACT		      = 1 << 0,
	EXPAND_ALL		  = 1 << 1
};

bool edit(const Serializer& ser, const char* stateFileName,
		  int flags = 0,
		  Widget* parent = 0, const char* title = 0);
bool edit(const Serializer& ser, const char* stateFileName,
		  int flags, HWND parent, const char* title = 0);

}

