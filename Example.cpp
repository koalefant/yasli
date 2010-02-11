#include <string>
#include <vector>

#include "TestData.h"
#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"
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

    Member()
    : weight(0.0f)
    , mode(MEMBER_MODE_A) {}

    Member(const char* _name, float _weight, MemberMode _mode)
    : name(_name)
    , weight(_weight)
    , mode(_mode) {}

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

        members_.push_back(Member("Item A", 0.1f, MEMBER_MODE_A));
        members_.push_back(Member("Item B", 0.3f, MEMBER_MODE_B));
        members_.push_back(Member("Item C", 0.6f, MEMBER_MODE_C));
        polyVector_.push_back( new PolyBase() );
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

YASLI_CLASS(PolyBase, PolyBase, "Base")
YASLI_CLASS(PolyBase, PolyDerivedA, "Derived A")

int main(int argc, char** argv)
{
    MyDataClass object;
    const char* filename = "example.ta";

    // let's read it
    TextIArchive ia;
    if(ia.load(filename))
        ia(object, "root");

    // and write back
    TextOArchive oa;
    oa(object, "root");
    oa.save(filename);
    return 0;
}
