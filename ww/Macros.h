/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Assert.h"

#define FOR_EACH(container, it) for( it = (container).begin(); it != (container).end(); ++it )

template<class T, size_t Len>
char (&globalArrayLenHelper(const T(&)[Len]))[Len];
#define ARRAY_LEN(arr) sizeof(globalArrayLenHelper(arr)) 

#ifndef NDEBUG
# define WW_VERIFY(x) ASSERT(x)
#else
# define WW_VERIFY(x) (x)
#endif

#undef min
#undef max

inline int min(int x,int y){ return x < y ? x : y; }
inline float min(float x,float y){ return x < y ? x : y; }
inline double min(double x,double y){ return x < y ? x : y; }

inline int max(int x,int y){ return x > y ? x : y; }
inline float max(float x,float y){ return x > y ? x : y; }
inline double max(double x,double y){ return x > y ? x : y; }
