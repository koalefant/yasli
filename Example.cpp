#include <string>
#include <vector>
#include "TestData.h"

int main(int argc, char** argv){
	TextOArchive oa;
	oa.open("object.ta"); // filename here
	MyDataClass object;
	oa(object, "object");
	// now check the "object.ta" text file =)
	return 0;
}
