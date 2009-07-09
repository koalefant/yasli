#pragma once

#include "yasli/Assert.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/PointersImpl.h"
#include "yasli/STLImpl.h"
#include "yasli/TextOArchive.h"

#include "yasli/StringList.h"

using namespace yasli;

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

class PolyBase : public RefCounter
{
public:
	PolyBase()
	{

	}

	virtual void serialize(Archive& ar)
	{		
		ar(baseMember_, "baseMember");
	}
protected:
	std::string baseMember_;
};

class PolyDerivedA : public PolyBase
{
public:
	void serialize(Archive& ar)
	{		
		PolyBase::serialize(ar);
		ar(derivedMember_, "derivedMember");
	}
protected:
	std::string derivedMember_;
};


class MyDataClass{
public:
	MyDataClass()
		: index_(0)
	{
		name_ = "Foo";
		stringList_.push_back("Choice 1");
		stringList_.push_back("Choice 2");
		stringList_.push_back("Choice 3");
		stringList_.push_back("Choice 4");
		stringList_.push_back("Choice 5");
		stringList_.push_back("Choice 6");
		stringList_.push_back("Choice 7");
		stringList_.push_back("Choice 8");

		polyPtr_.set( new PolyDerivedA() );
		
		/*
		members_.push_back(1.0f);
		memberVector_.push_back(2.0f);
		memberVector_.push_back(3.1415926f);
		*/
	}
	void serialize(Archive& ar){
		ar(name_, "name");
		ar(polyPtr_, "polyPtr");
		ar(polyVector_, "polyVector");
		ar(members_, "members");
		StringListValue value(stringList_, stringList_[index_]);
		ar(value, "stringList");
		index_ = value.index();
		if(index_ == -1)
			index_ = 0;
	}
protected:
	std::string name_;
	typedef std::vector<Member> Members;
	Members members_;
	int index_;
	StringListStatic stringList_;
	std::vector< SharedPtr<PolyBase> > polyVector_;
	SharedPtr<PolyBase> polyPtr_;
};

