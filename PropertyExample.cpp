#include "PropertyEdit.h"
#include "TestData.h"

int main(int argc, char** argv)
{
	MyDataClass data;
	propertyEdit(Serializer(data), 0);
}
