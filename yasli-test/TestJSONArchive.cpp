// This files contains strings both in utf-8 and windows-1251 encoding.
// We need this for CheckUtf8Conversion test.
#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/JSONIArchive.h"
#include "yasli/JSONOArchive.h"

#include <vector>
using std::vector;

#ifndef _MSC_VER
# include <wchar.h>
#endif

using std::string;
using namespace yasli;

SUITE(JSONArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		string bufChanged;
		ComplexClass objChanged;
		objChanged.change();
		{
			JSONOArchive oa;
			CHECK(oa(objChanged, "obj"));

			bufChanged = oa.c_str();
			CHECK(!bufChanged.empty());
		}

		string bufResaved;
		{
			ComplexClass obj;

			JSONIArchive ia;
			CHECK(ia.open(bufChanged.c_str(), bufChanged.size()));
			CHECK(ia(obj, "obj"));

			JSONOArchive oa;
			CHECK(oa(obj, "obj"));

			obj.checkEquality(objChanged);

			bufResaved = oa.c_str();
			CHECK(!bufChanged.empty());
		}
		CHECK_EQUAL(bufChanged, bufResaved);
	}

	TEST(RegressionFreezeReadingStructureAsContainer)
	{
		struct Element
		{
			bool enabled;
			string name;
			void serialize(Archive& ar)
			{
				ar(enabled, "enabled");
				ar(name, "name");
			}
		};

		struct Root
		{
			vector<Element> elements;
			void serialize(Archive& ar)
			{
				ar(elements, "elements");
			}
		};

		const char* content = 
		"[\n"
		"\t{ \"enabled\": true, \"name\": \"test\" },\n"
		"\t{ \"enabled\": true, \"name\": \"test\" }\n"
		"]";

		JSONIArchive ia;
		ia.open(content, strlen(content));

		Root obj;
		UNITTEST_TIME_CONSTRAINT(5);
		ia(obj, "");
	}
}

