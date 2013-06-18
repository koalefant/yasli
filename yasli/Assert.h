/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once 
#include <stdio.h>

namespace yasli{
void setTestMode(bool interactive);
}

#ifndef NDEBUG
namespace yasli{
void setInteractiveAssertion(bool interactive);
bool assertionDialog(const char* function, const char* fileName, int line, const char* expr, const char* str, ...);
inline bool assertionDialog(const char* function, const char* fileName, int line, const char* expr) { return assertionDialog(function, fileName, line, expr, ""); }
}
// if(YASLI_ASSERT(expr, "Message %i", 10)
#define YASLI_ASSERT(expr, ...) ((expr) || (yasli::assertionDialog(__FUNCTION__, __FILE__, __LINE__, #expr, __VA_ARGS__) ? __debugbreak(), false : false))
#define YASLI_CHECK YASLI_ASSERT
#else
# define YASLI_ASSERT(x)
#define YASLI_CHECK(expr, ...) (expr)
#endif

// use YASLI_CHECK instead
#define YASLI_ESCAPE(x, action) if(!(x)) { YASLI_ASSERT(0 && #x); action; } 

#pragma warning(disable:4127)
