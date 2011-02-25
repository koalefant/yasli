#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/TextIArchive.h"
#include "yasli/TextOArchive.h"

#ifndef _MSC_VER
# include <wchar.h>
#endif

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
		CHECK_EQUAL(bufChanged, bufResaved);
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
		const char* input = 
		"valu = \"Val1\"\n"
		"value = \"Val2\"\n";

		TextIArchive ia;
		CHECK(ia.open(input, strlen(input)));

		string value1;
		string value2;

		CHECK(ia(value1, "valu"));
		CHECK(ia(value2, "value"));

		CHECK_EQUAL("Val1", value1);
		CHECK_EQUAL("Val2", value2);
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

	TEST(RegressionNegativeFloat)
	{
		const char* input = 
		"value = -123.23\n";

		TextIArchive ia;
		CHECK(ia.open(input, strlen(input)));

		float value;
		CHECK(ia(value, "value"));
		CHECK_CLOSE(-123.23f, value, 0.001f);
	}

	TEST(RegressionTwoUnkownNameFreeze)
	{
		const char* input = 
		"known_value = \"Val\"\n"
		"unknown_name1 = 10\n"
		"unknown_name2 = 20\n"
		;

		TextIArchive ia;
		CHECK(ia.open(input, strlen(input)));
		string value;
		CHECK(ia(value, "known_value") == true);
		CHECK(ia(value, "known_value2") == false);
	}

	TEST(CheckUtfConversion)
  {
    {
      const char* input = "value = \"Кириллица\"\n";

      std::wstring value;

      TextIArchive ia;
      CHECK(ia.open(input, strlen(input)));
      CHECK(ia(value, "value") == true);
      CHECK(value == L"Кириллица");
    }

    {
      std::wstring value = L"\x041a\x0438";
      //wprintf(L"%ls\n", value.c_str());
      TextOArchive oa;
      CHECK(oa(value, "value"));
      //printf("hey: %s\n", oa.c_str());

      TextIArchive ia;
      CHECK(ia.open(oa.c_str(), oa.length()));

      std::wstring value2;
      CHECK(ia(value2, "value"));
      CHECK(value == value2);
    }

  }
}

