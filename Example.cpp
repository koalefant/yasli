#include <string>
#include <vector>
#include <sys/stat.h>
#include <windows.h>

#include "TestData.h"
#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"
#include "yasli/InPlaceOArchive.h"
#include "yasli/InPlaceIArchive.h"
using namespace yasli;

#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/PointersImpl.h"
#include "yasli/STLImpl.h"
#include "yasli/TypesFactory.h"

using namespace yasli;

enum MemberMode{
	MEMBER_MODE_A,
	MEMBER_MODE_B,
	MEMBER_MODE_C
};

YASLI_ENUM_BEGIN(MemberMode, "MemberMode")
YASLI_ENUM_VALUE(MEMBER_MODE_A, "Mode A")
YASLI_ENUM_VALUE(MEMBER_MODE_B, "Mode B")
YASLI_ENUM_VALUE(MEMBER_MODE_C, "Mode C")
YASLI_ENUM_END()

struct Member{
	std::string name;
	float weight;
	MemberMode mode;
	char memberWord[8];

	Member()
    : weight(rand() * 100.0f / RAND_MAX)
	, mode(MEMBER_MODE_A) 
	{
		strcpy(memberWord, "memberW");
		name = "member_name";
	}

	Member(const char* _name, float _weight, MemberMode _mode)
	: name(_name)
	, weight(_weight)
	, mode(_mode) 
	{
		strcpy(memberWord, "memberW");
	}

	void serialize(Archive& ar)
	{
		ar(name, "name");
		ar(weight, "weight");
		ar(mode, "mode");
	}
};

class PolyBase : public RefCounter
{
public:
	PolyBase()
	{
        baseMember_= "Very nice\nand long\nMember";
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
		// we are going to fill in default values
		name_ = "Foo, that is a name long enough to be allocated on heap";
		stringList_.push_back("Choice 1");
		stringList_.push_back("Choice 2");
		stringList_.push_back("Choice 3");
		stringList_.push_back("Choice 4");
		stringList_.push_back("Choice 5");
		stringList_.push_back("Choice 6");
		stringList_.push_back("Choice 7");
		stringList_.push_back("Choice 8");

		polyPtr_.set( new PolyDerivedA() );

		members_.push_back(Member("Item A", 0.1f, MEMBER_MODE_A));
		members_.push_back(Member("Item B", 0.3f, MEMBER_MODE_B));
		members_.push_back(Member("Item C", 0.6f, MEMBER_MODE_C));
		floatValues_.resize(200, 0.0f);
		for(size_t i = 0; i < floatValues_.size(); ++i)
			floatValues_[i] = rand() * 1000.0f / RAND_MAX;
		polyVector_.push_back( new PolyBase() );

		strcpy_s(endBuf_, "END");
	}
	void serialize(Archive& ar){
		ar(name_, "name");
		ar(polyPtr_, "polyPtr");
		//ar(polyVector_, "polyVector");
		//ar(members_, "members");

		if(!ar.isInPlace())
		{
			size_t s = floatValues_.size();
			ar(s, "numFloatValues");
			if(ar.isInput())
				floatValues_.reserve(s);
			//ar(floatValues_, "floatValues");

			StringListValue value(stringList_, stringList_[index_]);
			ar(value, "stringList");
			index_ = value.index();
			if(index_ == -1)
				index_ = 0;
		}
		else
		{
			//ar(floatValues_, "floatValues");
			ar(index_, "stringList");
		}
	}
protected:
	std::string name_;
	typedef std::vector<Member> Members;
	Members members_;
	int index_;
	StringListStatic stringList_;
	std::vector< SharedPtr<PolyBase> > polyVector_;
	std::vector< float > floatValues_;
	SharedPtr<PolyBase> polyPtr_;
	char endBuf_[4];
};

struct MyDataContainer
{
	MyDataContainer()
	{
		objects.resize(10);
	}

	void serialize(Archive& ar)
	{
		ar(objects, "objects");
	}
	std::vector<MyDataClass> objects;
};

YASLI_CLASS(PolyBase, PolyBase, "Base")
YASLI_CLASS(PolyBase, PolyDerivedA, "Derived A")

struct AutoTimer
{
	unsigned int startTime_;
	const char* name_;

	AutoTimer(const char* name)
	: name_(name)
	{
		startTime_ = GetTickCount();
	}

	int result() const
	{
		return GetTickCount() - startTime_;
	}

	~AutoTimer()
	{
		printf("%s: %i\n", name_, result());
	}
};

int getFileSize(const char* filename)
{
	struct _stat64 desc;
	if (_stat64(filename, &desc) == 0)
	{
		return (int)desc.st_size;
	}
	return -1;
}

void benchmark()
{
	const char* filename = "benchmark.inp";
	// and write back
	if(true)
	{
		InPlaceOArchive oa;
		MyDataContainer container;
		AutoTimer t("write time");
		oa(container);
		oa.save(filename);
	}

	size_t fileSize = getFileSize(filename);
	const int iterations = 1;
	{
		AutoTimer t("read time");
		for(int i = 0; i < iterations; ++i)
		{

			// let's read it
			InPlaceIArchive ia;
			const MyDataContainer* loaded = 0;
			{
				loaded = ia.load<MyDataContainer>(filename);
			}

			if(loaded)
			{
				free((void*)loaded);
				//ia(objects, "objects");
			}
		}
		printf("speed: %.2f MB/s\n", float(fileSize * iterations) / (1024 * 1024) / (float(t.result() / 1000.0f)));
	}
}

void testInplace()
{
	const char* inplaceFilename = "test.inp";
	{
		std::auto_ptr<MyDataClass> test(new MyDataClass());
		InPlaceOArchive oa;
		ESCAPE(oa(*test, "test"), return);
		oa.save(inplaceFilename);
	}

	InPlaceIArchive ia;
	const MyDataClass* result = ia.load<MyDataClass>(inplaceFilename);
	if(result)
	{
		free((void*)(result));
	}
}

int main(int argc, char** argv)
{
	benchmark();
	//testInplace();
	
	return 0;
}
