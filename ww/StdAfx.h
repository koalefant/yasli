#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>

#define _WIN32_WINNT 0x0501

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <list>
#include "yasli/Pointers.h"
#include "XMath/MinMax.h"
#include "XMath/Profiler.h"

#pragma warning(disable: 4355) //  'this' : used in base member initializer list

#define FOR_EACH(container, it) for( it = (container).begin(); it != (container).end(); ++it )

template<class T, size_t Len>
char (&globalArrayLenHelper(const T(&)[Len]))[Len];
#define ARRAY_LEN(arr) sizeof(globalArrayLenHelper(arr)) 

