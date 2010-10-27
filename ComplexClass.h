#pragma once

#include "yasli/Assert.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/PointersImpl.h"
#include "yasli/STLImpl.h"
#include "yasli/TextOArchive.h"
#include "yasli/Macros.h"

#include "yasli/StringList.h" 
using namespace yasli;

struct Member
{
	std::string name;
	float weight;

	Member()
	: weight(0.0f) {}

	void checkEquality(const Member& copy) const
	{
		CHECK(name == copy.name);
		CHECK(weight == copy.weight);
	}

	void change(int index)
	{
		name = "Changed name ";
		name += (index % 10) + '0';
		weight = float(index);
	}

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
		baseMember_ = "Regular base member";
	}

	virtual void change()
	{
		baseMember_ = "Changed base member";
	}

	virtual void serialize(Archive& ar)
	{		
		ar(baseMember_, "baseMember");
	}

	virtual void checkEquality(const PolyBase* copy) const
	{
		CHECK(baseMember_ == copy->baseMember_);
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

	void checkEquality(const PolyBase* copyBase) const
	{
		const PolyDerivedA* copy = dynamic_cast<const PolyDerivedA*>(copyBase);
		CHECK(copy != 0);
		CHECK(derivedMember_ == copy->derivedMember_);

		PolyBase::checkEquality(copyBase);
	}
protected:
	std::string derivedMember_;
};

class PolyDerivedB : public PolyBase
{
public:
	PolyDerivedB()
	: derivedMember_("B Derived")
	{
	}

	void serialize(Archive& ar)
	{		
		PolyBase::serialize(ar);
		ar(derivedMember_, "derivedMember");
	}

	void checkEquality(const PolyBase* copyBase) const
	{
		const PolyDerivedB* copy = dynamic_cast<const PolyDerivedB*>(copyBase);
		CHECK(copy != 0);
		CHECK(derivedMember_ == copy->derivedMember_);

		PolyBase::checkEquality(copyBase);
	}
protected:
	std::string derivedMember_;
};


class ComplexClass{
public:
	ComplexClass()
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

		polyVector_.push_back(new PolyDerivedB);
		polyVector_.push_back(new PolyBase);

		members_.resize(13);
	}

	void change()
	{
		name_ = "Slightly changed name";
		index_ = 2;
		polyPtr_.set( new PolyDerivedB() );
		polyPtr_->change();

		for (size_t i = 0; i < members_.size(); ++i)
			members_[i].change(i);

		for (size_t i = 0; i < polyVector_.size(); ++i)
			polyVector_[i]->change();

		polyVector_.resize(4);
		polyVector_.push_back(new PolyBase());
		polyVector_[4]->change();

		for (size_t i = 0; i < ARRAY_LEN(array_); ++i)
			array_[i].change(ARRAY_LEN(array_) - i);
	}

	void serialize(Archive& ar)
	{
		ar(name_, "name");
		ar(polyPtr_, "polyPtr");
		ar(polyVector_, "polyVector");
		ar(members_, "members");
		if(ar.isInPlace())
		{
			ar(index_, "index");
		}
		else
		{
			StringListValue value(stringList_, stringList_[index_]);
			ar(value, "stringList");
			index_ = value.index();
			if(index_ == -1)
				index_ = 0;
		}
		ar(array_, "array");
	}

	void checkEquality(const ComplexClass& copy) const
	{
		CHECK(name_ == copy.name_);
		CHECK(index_ == copy.index_);

		CHECK(polyPtr_ != 0);
		CHECK(copy.polyPtr_ != 0);
		polyPtr_->checkEquality(copy.polyPtr_);

		CHECK(members_.size() == copy.members_.size());
		for (size_t i = 0; i < members_.size(); ++i)
		{
			members_[i].checkEquality(copy.members_[i]);
		}

		CHECK(polyVector_.size() == copy.polyVector_.size());
		for (size_t i = 0; i < polyVector_.size(); ++i)
		{
			if(polyVector_[i] == 0)
			{
				CHECK(copy.polyVector_[i] == 0);
				continue;
			}
			CHECK(copy.polyVector_[i] != 0);
			polyVector_[i]->checkEquality(copy.polyVector_[i]);
		}

		for (size_t i = 0; i < ARRAY_LEN(array_); ++i)
		{
			array_[i].checkEquality(copy.array_[i]);
		}
	}
protected:
	std::string name_;
	typedef std::vector<Member> Members;
	Members members_;
	int index_;
	StringListStatic stringList_;
	std::vector< SharedPtr<PolyBase> > polyVector_;
	SharedPtr<PolyBase> polyPtr_;

	Member array_[5];
};

