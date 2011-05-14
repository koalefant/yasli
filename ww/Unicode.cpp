#include "StdAfx.h"
#include "Unicode.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ww{

std::string fromWideChar(const wchar_t* wstr)
{
#ifdef WW_DISABLE_UTF8
	const unsigned int codepage = CP_ACP;
#else
	const unsigned int codepage = CP_UTF8;
#endif
	int len = WideCharToMultiByte(codepage, 0, wstr, -1, NULL, 0, 0, 0);
	char* buf = (char*)alloca(len);
    if(len > 1){ 
        WideCharToMultiByte(codepage, 0, wstr, -1, buf, len, 0, 0);
		return std::string(buf, len - 1);
    }
	return std::string();

}

std::wstring toWideChar(const char* str)
{
#ifdef WW_DISABLE_UTF8
	const unsigned int codepage = CP_ACP;
#else
	const unsigned int codepage = CP_UTF8;
#endif
    int len = MultiByteToWideChar(codepage, 0, str, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)alloca(len * sizeof(wchar_t));
    if(len > 1){ 
        MultiByteToWideChar(codepage, 0, str, -1, buf, len);
		return std::wstring(buf, len - 1);
    }
	return std::wstring();
}

std::wstring fromANSIToWide(const char* str)
{
	const unsigned int codepage = CP_ACP;
    int len = MultiByteToWideChar(codepage, 0, str, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)alloca(len * sizeof(wchar_t));
    if(len > 1){ 
        MultiByteToWideChar(codepage, 0, str, -1, buf, len);
		return std::wstring(buf, len - 1);
    }
	return std::wstring();
}

std::string toANSIFromWide(const wchar_t* wstr)
{
	const unsigned int codepage = CP_ACP;
	int len = WideCharToMultiByte(codepage, 0, wstr, -1, NULL, 0, 0, 0);
	char* buf = (char*)alloca(len);
    if(len > 1){ 
        WideCharToMultiByte(codepage, 0, wstr, -1, buf, len, 0, 0);
		return std::string(buf, len - 1);
    }
	return std::string();

}


FILE* fopen(const char* utf8filename, const char* mode)
{
#ifdef WIN32
    return _wfopen(toWideChar(utf8filename).c_str(), toWideChar(mode).c_str());
#else
    return fopen(utf8filename, mode);
#endif
}

}
