#undef new
#undef delete
#undef calloc
#undef malloc
#undef realloc
#undef free

#ifdef WIN32
# include <windows.h>
# pragma warning(disable: 4250)
#endif

#include "utils/Pointers.h"
#include "utils/Errors.h"
#include "utils/SafeCast.h"
#include "utils/Macros.h"

#include <vector>
#include <list>
#include <map>
#include <string>
#include <sstream>

#undef min
#undef max

