/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/API.h"

namespace ww{
	class ConstStringWrapper;
};

bool serialize(yasli::Archive& ar, ww::ConstStringWrapper &wrapper, const char* name, const char* label);

namespace ww{

class ConstStringList{
public:
	const char* findOrAdd(const char* string);
protected:
	typedef std::list<std::string> Strings;
	Strings strings_;
};

class ConstStringWrapper{
public:
	ConstStringWrapper(ConstStringList* list, const char*& string);
protected:
	ConstStringList* list_;
	const char*& string_;
	friend bool ::serialize(Archive& ar, ConstStringWrapper &wrapper, const char* name, const char* label);
};

extern ConstStringList globalConstStringList;

}


