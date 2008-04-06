#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"
#include "yasli/Files.h"
#include "property-tree/PropertyEdit.h"
#include "TestData.h"

int main(int argc, char** argv)
{
	MyDataClass data;
	if(Files::exists("test.ta")){
		TextIArchive ia;
		ia.open("test.ta");
		ia(data, "data");
	}
	propertyEdit(Serializer(data), 0);
	TextOArchive oa;
	oa.open("test.ta");
	oa(data, "data");
}
