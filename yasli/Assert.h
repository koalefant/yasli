#pragma once 
#include <stdio.h>

#ifdef ASSERT
# undef ASSERT
#endif

#ifdef VERIFY
# undef VERIFY
#endif

#ifdef ESCAPE
# undef ESCAPE
#endif

#ifndef NDEBUG
namespace yasli{
void setInteractiveAssertion(bool interactive);
int assertionDialog(const char* message, const char* str, const char* function, const char* fileName, int line);
}
# ifdef WIN32
#  define ASSERT(x) \
      { \
          static bool ignore = false; \
          if(!(x) && !ignore) \
		  switch(yasli::assertionDialog((#x), 0, __FUNCTION__, __FILE__, __LINE__)) {  \
          case 0: break; \
          case 1: ignore = true; break;  \
          case 2: __asm{ int 3 } break;  \
          }  \
      } 
#  define ASSERT_STR(x, str) \
      { \
          static bool ignore = false; \
          if(!(x) && !ignore) \
		  switch(yasli::assertionDialog((#x), (str), __FUNCTION__, __FILE__, __LINE__)) {  \
          case 0: break; \
          case 1: ignore = true; break;  \
          case 2: __asm{ int 3 } break;  \
          }  \
      } 
# else
#  define ASSERT(x) { bool val = (x); if (!val) { fprintf(stderr, __FILE__ ":%i: assertion: " #x "\n\tin %s()\n", __LINE__, __FUNCTION__); }  }
#  define ASSERT_STR(x, str) { bool val = (x); if (!val) { fprintf(stderr, __FILE__ ":%i: assertion: " #x " (%s)\n\tin %s()\n", __LINE__, str, __FUNCTION__); }  }
# endif
#else
# define ASSERT(x)
# define ASSERT_STR(x, str)
#endif

#define ESCAPE(x, action) if(!(x)) { ASSERT(0 && #x); action; };
