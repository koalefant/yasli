#include <string>
#include <vector>

#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"
#include "yasli/TextOArchive.h"

class MyDataClass{
public:
	MyDataClass(){
		name_ = "Foo";
		memberVector_.push_back(1.0f);
		memberVector_.push_back(2.0f);
		memberVector_.push_back(3.1415926f);
	}
	void serialize(Archive& ar){
		ar(name_, "name");
		ar(memberVector_, "memberVector");
	}
protected:
	std::string name_;
	std::vector<float> memberVector_;
};

int main(int argc, char** argv){
	TextOArchive oa;
	oa.open("object.ta"); // filename here
	MyDataClass object;
	oa(object, "object");
	// now check the "object.ta" text file =)
	return 0;
}
