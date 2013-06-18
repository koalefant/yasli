/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "MemoryWriter.h"
#include "BinArchive.h"

namespace yasli{

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#ifndef NDEBUG
static bool interactiveAssertion = true;
static bool testMode = true;

void setInteractiveAssertion(bool interactive)
{
    interactiveAssertion = interactive;
}

void setTestMode(bool testMode)
{
	interactiveAssertion = false;
    testMode = testMode;
}

bool assertionDialog(const char* function, const char* fileName, int line, const char* expr, const char* str, ...)
{
	MemoryWriter text;
	text << fileName << "(" << line << "): error: " << "Assertion in " << function << "(): " << expr; // output in msvc error format

#ifdef WIN32
	int hash = calcHash(text.c_str());

	const int hashesMax = 1000;
	static int hashes[hashesMax];
	int index = 0;
	for(; index < hashesMax; ++index)
		if(hashes[index] == hash)
			return false;
		else if(!hashes[index])
			break;
#endif

	char buffer[4000];
	va_list args;
	va_start(args, str);
	vsprintf_s(buffer, 4000, str, args);
	va_end(args);

	text << " ( " << buffer << " )";
	text << "\n";

#ifdef WIN32
    if(interactiveAssertion || IsDebuggerPresent()){
        int result = MessageBoxA(0, text.c_str(), "Debug Assertion Triggered", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_TASKMODAL);
        switch(result){
        case IDRETRY: 
			return false;
        case IDIGNORE: 
			hashes[index] = hash;
			return false;
        case IDABORT: 
			return true;
        }
    }
    else{
        fwrite(text.c_str(), text.size(), 1, stderr);
		fflush(stderr);
		if(testMode)
			exit(-1);
    }
#else
	fprintf(stderr, text.c_str()); 
#endif
	return false; 
}

#endif

}
