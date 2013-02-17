/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
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
int assertionDialog(const char* message, const char* str, const char* function, const char* fileName, int line);
}
# ifdef WIN32
#  define YASLI_ASSERT(x) \
      { \
          static bool ignore = false; \
          if(!(x) && !ignore) \
		  switch(yasli::assertionDialog((#x), 0, __FUNCTION__, __FILE__, __LINE__)) {  \
          case 0: break; \
          case 1: ignore = true; break;  \
          case 2: __debugbreak(); break;  \
          }  \
      } 
#  define YASLI_ASSERT_STR(x, str) \
      { \
          static bool ignore = false; \
          if(!(x) && !ignore) \
		  switch(yasli::assertionDialog((#x), (str), __FUNCTION__, __FILE__, __LINE__)) {  \
          case 0: break; \
          case 1: ignore = true; break;  \
          case 2: __debugbreak(); break;  \
          }  \
      } 
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