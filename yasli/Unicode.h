#pragma once
#include <string>

namespace yasli{
	std::string fromWideChar(const wchar_t* wideCharString);
	std::wstring toWideChar(const char* multiByteString);
	std::wstring fromANSIToWide(const char* ansiString);
    std::string toANSIFromWide(const wchar_t* wstr);

    FILE* fopen(const char* utf8filename, const char* mode);
}
