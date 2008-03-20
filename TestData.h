#pragma once

#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"
#include "yasli/TextOArchive.h"

struct Member{
	std::string name;
	float weight;

	Member()
	: weight(0.0f) {}

	void serialize(Archive& ar)
	{
		ar(name, "name");
		ar(weight, "weight");
	}
};

class MyDataClass{
public:
	MyDataClass(){
		name_ = "Foo";
		/*
		members_.push_back(1.0f);
		memberVector_.push_back(2.0f);
		memberVector_.push_back(3.1415926f);
		*/
	}
	void serialize(Archive& ar){
		ar(name_, "name");
		ar(members_, "members");
	}
protected:
	std::string name_;
	typedef std::vector<Member> Members;
	Members members_;
};
