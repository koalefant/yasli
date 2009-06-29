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

//#include <wx/wx.h>

#include "yasli/Pointers.h"
#include "yasli/Assert.h"
#include "yasli/SafeCast.h"
#include "yasli/Macros.h"

#include <vector>
#include <list>
#include <map>
#include <string>
#include <sstream>

#undef min
#undef max

