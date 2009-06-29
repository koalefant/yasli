#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"
#include "yasli/Files.h"
#include "property-tree/PropertyEdit.h"
#include "TestData.h"

int main(int argc, char** argv)
{
	MyDataClass data;
	TextIArchive ia;
	if(ia.load("test.ta"))
		ia(data, "data");

	propertyEdit(Serializer(data), 0);
	TextOArchive oa;
	oa(data, "data");
	oa.save("test.ta");
}
