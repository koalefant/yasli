#pragma once
#include <string>
#include <stdio.h>

namespace ww{
	using std::string;
	using std::wstring;

	string fromWideChar(const wchar_t* wideCharString);
	wstring toWideChar(const char* multiByteString);
	wstring fromANSIToWide(const char* ansiString);
    string toANSIFromWide(const wchar_t* wstr);

    FILE* fopen(const char* utf8filename, const char* mode);
}
