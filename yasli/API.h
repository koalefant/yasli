#pragma once

#ifdef _MSC_VER
# pragma warning(disable: 4251 4275)
#endif

#ifdef WIN32
# ifdef USE_SHARED_LIBS
#  define EXPORT_SYMBOL __declspec(dllexport)
#  define IMPORT_SYMBOL __declspec(dllimport)
# else
#  define EXPORT_SYMBOL
#  define IMPORT_SYMBOL
# endif
#else
# define EXPORT_SYMBOL
# define IMPORT_SYMBOL
#endif

#ifdef YASLI_EXPORTED
# define YASLI_API EXPORT_SYMBOL
#else
# define YASLI_API IMPORT_SYMBOL
#endif
