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

#ifdef YASLI_ASSERT
# undef YASLI_ASSERT
#endif

#ifdef YASLI_VERIFY
# undef YASLI_VERIFY
#endif

#ifdef YASLI_ESCAPE
# undef YASLI_ESCAPE
#endif

namespace yasli{
void setTestMode(bool interactive);
}

#ifndef NDEBUG
namespace yasli{
void setInteractiveAssertion(bool interactive);
bool assertionDialog(const char* function, const char* fileName, int line, const char* expr, const char* str, ...);
inline bool assertionDialog(const char* function, const char* fileName, int line, const char* expr) { return assertionDialog(function, fileName, line, expr, ""); }
}
#ifdef WIN32
#define YASLI_ASSERT(expr, ...) ((expr) || (yasli::assertionDialog(__FUNCTION__, __FILE__, __LINE__, #expr, __VA_ARGS__) ? __debugbreak(), false : false))
#define YASLI_ASSERT_STR(expr, str) YASLI_ASSERT(expr, str)
# else
#  define YASLI_ASSERT(x) { bool val = (x); if (!val) { fprintf(stderr, __FILE__ ":%i: assertion: " #x "\n\tin %s()\n", __LINE__, __FUNCTION__); }  }
#  define YASLI_ASSERT_STR(x, str) { bool val = (x); if (!val) { fprintf(stderr, __FILE__ ":%i: assertion: " #x " (%s)\n\tin %s()\n", __LINE__, str, __FUNCTION__); }  }
# endif
#else
# define YASLI_ASSERT(x)
# define YASLI_ASSERT_STR(x, str)
#endif

#define YASLI_ESCAPE(x, action) if(!(x)) { YASLI_ASSERT(0 && #x); action; };

#pragma warning(disable:4127)
