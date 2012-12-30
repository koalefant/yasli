/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "Unicode.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

yasli::string fromWideChar(const wchar_t* wstr)
{
	const unsigned int codepage = CP_ACP;
	int len = WideCharToMultiByte(codepage, 0, wstr, -1, NULL, 0, 0, 0);
	char* buf = (char*)alloca(len);
    if(len > 1){ 
        WideCharToMultiByte(codepage, 0, wstr, -1, buf, len, 0, 0);
		return yasli::string(buf, len - 1);
    }
	return yasli::string();

}

yasli::wstring toWideChar(const char* str)
{
	const unsigned int codepage = CP_ACP;
    int len = MultiByteToWideChar(codepage, 0, str, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)alloca(len * sizeof(wchar_t));
    if(len > 1){ 
        MultiByteToWideChar(codepage, 0, str, -1, buf, len);
		return yasli::wstring(buf, len - 1);
    }
	return yasli::wstring();
}

yasli::wstring fromANSIToWide(const char* str)
{
	const unsigned int codepage = CP_ACP;
    int len = MultiByteToWideChar(codepage, 0, str, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)alloca(len * sizeof(wchar_t));
    if(len > 1){ 
        MultiByteToWideChar(codepage, 0, str, -1, buf, len);
		return yasli::wstring(buf, len - 1);
    }
	return yasli::wstring();
}

yasli::string toANSIFromWide(const wchar_t* wstr)
{
	const unsigned int codepage = CP_ACP;
	int len = WideCharToMultiByte(codepage, 0, wstr, -1, NULL, 0, 0, 0);
	char* buf = (char*)alloca(len);
    if(len > 1){ 
        WideCharToMultiByte(codepage, 0, wstr, -1, buf, len, 0, 0);
		return yasli::string(buf, len - 1);
    }
	return yasli::string();

}

