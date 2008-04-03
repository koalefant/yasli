#pragma once

#ifdef WIN32

#ifdef USE_SHARED_LIBS
# define EXPORT_SYMBOL __declspec(dllexport)
# define IMPORT_SYMBOL __declspec(dllimport)
#else
# define EXPORT_SYMBOL
# define IMPORT_SYMBOL
#endif

#else

# define EXPORT_SYMBOL
# define IMPORT_SYMBOL

#endif
