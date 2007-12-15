#ifndef __UTILS_API_H_INCLUDED__
#define __UTILS_API_H_INCLUDED__

#include "SharedLibApi.h"

#ifdef _MSC_VER
# pragma warning(disable: 4275)
#endif

#ifdef UTILS_EXPORTED
# define UTILS_API EXPORT_SYMBOL
#else
# define UTILS_API IMPORT_SYMBOL
#endif

#endif
