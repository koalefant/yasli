#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/InPlaceIArchive.h"
#include "yasli/InPlaceOArchive.h"

using std::string;
using namespace yasli;

SUITE(InPlaceArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		const char* inplaceFilename = "test.inp";
		{
			std::auto_ptr<ComplexClass> obj(new ComplexClass);
			obj->change();

			InPlaceOArchive oa(false);
			ESCAPE(oa(*obj, "obj"), return);
			oa.save(inplaceFilename);
		}

		ComplexClass objChanged;
		objChanged.change();
		{
			ComplexClass obj;

			InPlaceIArchive ia;
			const ComplexClass* result = ia.load<ComplexClass>(inplaceFilename);
			CHECK(result != 0);
			if (result)
			{
				result->checkEquality(objChanged);
				free((void*)(result));
			}
		}
	}
}

