#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/BinaryIArchive.h"
#include "yasli/BinaryOArchive.h"

#ifndef _MSC_VER
# include <wchar.h>
#endif

using std::string;
using namespace yasli;

SUITE(BinaryArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		ComplexClass objChanged;
		objChanged.change();
		BinaryOArchive oa;
		CHECK(oa(objChanged, "obj"));
		CHECK(oa.length() != 0);

		{
			ComplexClass obj;

			BinaryIArchive ia;
			CHECK(ia.open(oa.buffer(), oa.length()));
			CHECK(ia(obj, "obj"));

			BinaryOArchive oa2;
			CHECK(oa2(obj, "obj"));

			obj.checkEquality(objChanged);
			CHECK(oa2.length() != 0);
			CHECK(oa.length() == oa2.length());
			CHECK(memcmp(oa.buffer(), oa2.buffer(), oa.length()) == 0);
		}
	}
}
