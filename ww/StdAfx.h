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

