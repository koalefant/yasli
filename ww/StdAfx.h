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
#include "yasli/Pointers.h"

#ifndef WW_DISABLE_XMATH
# include "XMath/Profiler.h"
#endif

#pragma warning(disable: 4355) //  'this' : used in base member initializer list

#define FOR_EACH(container, it) for( it = (container).begin(); it != (container).end(); ++it )

template<class T, size_t Len>
char (&globalArrayLenHelper(const T(&)[Len]))[Len];
#define ARRAY_LEN(arr) sizeof(globalArrayLenHelper(arr)) 

inline int min(int x,int y){ return x < y ? x : y; }
inline float min(float x,float y){ return x < y ? x : y; }
inline double min(double x,double y){ return x < y ? x : y; }

inline int max(int x,int y){ return x > y ? x : y; }
inline float max(float x,float y){ return x > y ? x : y; }
inline double max(double x,double y){ return x > y ? x : y; }
