#pragma once 

#ifdef WIN32
#else
#endif

#ifndef NDEBUG
namespace yasli{
int assertionDialog(const char* message, const char* function, const char* fileName, int line);
}
# ifdef WIN32
#  define ASSERT(x) \
      { \
          static bool ignore = false; \
          if(!(x) && !ignore) \
		  switch(yasli::assertionDialog((#x), __FUNCTION__, __FILE__, __LINE__)) {  \
          case 0: break; \
          case 1: ignore = true; break;  \
          case 2: __asm{ int 3 } break;  \
          }  \
      } 
#  define ASSERT_STR(x, str) ASSERT(x)
#  define VERIFY(x) ASSERT(x);
# else
#  include <assert.h>
#  define ASSERT(x) assert((x))
#  define ASSERT_STR(x, str) assert((x))
#  define VERIFY(x) assert(x);
# endif
//# define TRACE(x) ErrorHelpers::trace((x))
#else
# define ASSERT(x)
# define ASSERT_STR(x, str)
# define VERIFY(x) (x)
//# define TRACE(x)
#endif
#define CHECK(x, action) if(!(x)) { ASSERT(0 && #x); action; };
