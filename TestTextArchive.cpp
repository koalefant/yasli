#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/TextIArchive.h"
#include "yasli/TextOArchive.h"

using std::string;
using namespace yasli;

SUITE(TextArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		string bufChanged;
		ComplexClass objChanged;
		objChanged.change();
		{
			TextOArchive oa;
			CHECK(oa(objChanged, "obj"));

			bufChanged = oa.c_str();
			CHECK(!bufChanged.empty());
		}

		string bufResaved;
		{
			ComplexClass obj;

			TextIArchive ia;
			CHECK(ia.open(bufChanged.c_str(), bufChanged.size()));

			CHECK(ia(obj, "obj"));

			TextOArchive oa;
			CHECK(oa(obj, "obj"));

			obj.checkEquality(objChanged);

			bufResaved = oa.c_str();
			CHECK(!bufChanged.empty());
		}
		CHECK(bufChanged == bufResaved);
	}

	TEST(RegressionEmptyFileFreeze)
	{
		const char* input =
		"# comment\n"
		"\n";

		TextIArchive ia;
		ia.open(input, strlen(input));

		string value;
		ia(value, "value");
		CHECK(true);
	}

	TEST(Regression4Or5LengthNames)
	{
		string value1 = "Value1";
		string value2 = "Value2";

		TextOArchive oa;
		oa(value1, "valu");
		oa(value2, "value");

		TextIArchive ia;
		CHECK(ia.open(oa.c_str(), oa.length()));
		string value1New;
		CHECK(ia(value1New, "valu"));
		string value2New;
		CHECK(ia(value2New, "value"));

		CHECK(value1 == value1New);
		CHECK(value2 == value2New);
	}

	TEST(RegressionSubstringName)
	{
		const char* input = 
		"known_value = \"Val\"\n";

		TextIArchive ia;
		CHECK(ia.open(input, strlen(input)));
		string value;
		CHECK(ia(value, "known_value2") == false);
	}

	TEST(RegressionTwoUnkownNameFreeze)
	{
		const char* input = 
		"unknown_name1 = 10\n"
		"unknown_name2 = 20\n"
		"known_value = \"Val\"\n"
		;

		TextIArchive ia;
		CHECK(ia.open(input, strlen(input)));
		string value;
		CHECK(ia(value, "known_value") == true);
		CHECK(ia(value, "known_value2") == false);
	}
}

