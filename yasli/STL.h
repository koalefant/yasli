/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include <list>

#include "yasli/Serializer.h"

namespace yasli{ class Archive; }

template<class T, class Alloc>
bool serialize(yasli::Archive& ar, std::vector<T, Alloc>& container, const char* name, const char* label);

template<class T, class Alloc>
bool serialize(yasli::Archive& ar, std::list<T, Alloc>& container, const char* name, const char* label);

bool serialize(yasli::Archive& ar, std::string& value, const char* name, const char* label);
bool serialize(yasli::Archive& ar, std::wstring& value, const char* name, const char* label);
