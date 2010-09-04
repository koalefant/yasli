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


