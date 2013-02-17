#include <memory>
#include <stdlib.h>
#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/InPlaceIArchive.h"
#include "yasli/InPlaceOArchive.h"

using std::string;
using namespace yasli;

#ifndef _WIN64 // InPlaceArchive is not ported yet to 64-bit
SUITE(InPlaceArchive)
{
	TEST(ComplexSaveAndLoad)
	{
#ifdef WIN32
#ifdef _WIN64
		const char* inplaceFilename = "test.inp64";
#else
		const char* inplaceFilename = "test.inp";
#endif
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
#endif
	}
}
#endif