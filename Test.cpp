#include "UnitTest++.h"
#include "ComplexClass.h"

YASLI_CLASS(PolyBase, PolyBase, "Base")
YASLI_CLASS(PolyBase, PolyDerivedA, "Derived A")
YASLI_CLASS(PolyBase, PolyDerivedB, "Derived B")


int main(int argc, char* argv[])
{
	return UnitTest::RunAllTests();
}
