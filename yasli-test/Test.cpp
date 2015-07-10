#include "UnitTest++.h"
#include "ComplexClass.h"

YASLI_CLASS_NAME(PolyBase, PolyBase, "base", "Base")
YASLI_CLASS_NAME(PolyBase, PolyDerivedA, "derivedA", "Derived A")
YASLI_CLASS_NAME(PolyBase, PolyDerivedB, "derivedB", "Derived B")


int main(int argc, char* argv[])
{
#ifndef NDEBUG
//	yasli::setTestMode(true);
#endif
	return UnitTest::RunAllTests();
}
