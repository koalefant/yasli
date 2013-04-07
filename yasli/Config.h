/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


#pragma once

// Disable C++ RTTI use (e.g. typeid())
#define YASLI_NO_RTTI 1

// Disable extra struct-level for polymorphic pointer serialization
#define YASLI_NO_EXTRA_BLOCK_FOR_POINTERS 0

// Default filter for Archive: 0 - strict mode, -1 - silent mode.
#define YASLI_DEFAULT_FILTER 0
