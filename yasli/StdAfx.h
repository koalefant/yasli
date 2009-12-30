#pragma once

#ifdef WIN32
# ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0500
# endif
# define WIN32_LEAN_AND_MEAN
#endif

#include <typeinfo>
#include <vector>
#include <list>
#include <map>
#include "stdio.h"

#include "Assert.h"

