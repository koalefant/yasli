/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Strings.h"
#include "ww/API.h"
#include "ww/PropertyRowImpl.h"

namespace yasli{
class EnumDescription;
}

namespace ww{

class PropertyTreeModel;

class PropertyRowString : public PropertyRowImpl<wstring, PropertyRowString>{
public:
	enum { Custom = false };
	bool assignTo(string& str);
	bool assignTo(wstring& str);
	void setValue(const char* str);
	void setValue(const wchar_t* str);
	PropertyRowWidget* createWidget(PropertyTree* tree);
	string valueAsString() const;
	wstring valueAsWString() const { return value_; }
};

}

