/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "ConstStringList.h"
#include <algorithm>
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

ConstStringList globalConstStringList;

const char* ConstStringList::findOrAdd(const char* string)
{
	// TODO: try sorted vector of const char*
	Strings::iterator it = std::find(strings_.begin(), strings_.end(), string);
	if(it == strings_.end()){
		strings_.push_back(string);
		return strings_.back().c_str();
	}
	else{
		return it->c_str();
	}
}


ConstStringWrapper::ConstStringWrapper(ConstStringList* list, const char*& string)
: list_(list ? list : &globalConstStringList)
, string_(string)
{
	YASLI_ASSERT(string_);
}


bool serialize(yasli::Archive& ar, ConstStringWrapper& value, const char* name, const char* label)
{
	using yasli::string;
	if(ar.isOutput()){
        YASLI_ASSERT(value.string_);
        string out = value.string_;
		return ar(out, name, label);
	}
	else{
		string in;
		bool result = ar(in, name, label);

        value.string_ = value.list_->findOrAdd(in.c_str());
		return result;
	}
}
