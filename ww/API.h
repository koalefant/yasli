/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#pragma warning(disable : 4251) // class '' needs to have dll-interface to be used by clients of class ''
#pragma warning(disable : 4275) // non dll-interface class '' used as base for dll-interface class ''
#pragma warning(disable : 4512) // assignment operator could not be generated

#if !defined(WW_DLL) && (defined(_AFXDLL) || defined(_DLL) || defined(_MTD))
#define WW_DLL
#endif

#ifndef WW_DLL
# define WW_API 
#else
# ifdef WW_EXPORTED
#  define WW_API __declspec(dllexport)
# else
#  define WW_API __declspec(dllimport)
# endif
#endif

namespace yasli{}

namespace ww{
    using namespace yasli;
}

