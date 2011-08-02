/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max

#include <string>
#include <vector>
#include <list>
#include "Macros.h"
#include "yasli/Pointers.h"

#ifndef WW_DISABLE_XMATH
# include "XMath/Profiler.h"
#endif

#pragma warning(disable: 4355) //  'this' : used in base member initializer list

