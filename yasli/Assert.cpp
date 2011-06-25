#include "StdAfx.h"
#include "MemoryWriter.h"

namespace yasli{

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

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

int assertionDialog(const char* message, const char* str, const char* function, const char* fileName, int line)
{
    if(interactiveAssertion || IsDebuggerPresent()){
        MemoryWriter text;
        text << "in " << function << "():\n";
        text << "\n    " << message;
        if(str){
            text << "\n    ( " << str << " )";
        }
        text << "\n\n";
        text << fileName << ": line " << line << "\n";
        int result = MessageBoxA(0, text.c_str(), "Debug Assertion Triggered", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_TASKMODAL);
        switch(result){
        case IDRETRY: return 0;
        case IDIGNORE: return 1;
        case IDABORT: return 2;
        }
        return 0;
    }
    else{
        MemoryWriter text;
		// output in msvc error format
		text << fileName << "(" << line << "): error: " << "Assertion in " << function << "(): " << message;
        if(str)
            text << " ( " << str << " )";
		text << "\n";
        fwrite(text.c_str(), text.size(), 1, stderr);
		fflush(stderr);
		if (testMode)
		{
			exit(-1);
		}
        return 0; // Retry
    }
}

#endif
#endif

}