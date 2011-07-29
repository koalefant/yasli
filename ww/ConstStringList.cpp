#include "StdAfx.h"
#include "ww/ConstStringList.h"
#include <algorithm>
#include "yasli/Archive.h"

namespace ww{

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
	ASSERT(string_);
}

}

bool serialize(yasli::Archive& ar, ww::ConstStringWrapper& val, const char* name, const char* label)
{
	if(ar.isOutput()){
		ASSERT(val.string_);
		std::string out = val.string_;
		return ar(out, name, label);
	}
	else{
		std::string in;
		bool result = ar(in, name, label);

		val.string_ = val.list_->findOrAdd(in.c_str());
		return result;
	}
}
