#include "StdAfx.h"
#include "MemoryWriter.h"

namespace yasli{

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

#ifndef NDEBUG
static bool interactiveAssertion = true;

void setInteractiveAssertion(bool interactive)
{
    interactiveAssertion = interactive;
}

int assertionDialog(const char* message, const char* str, const char* function, const char* fileName, int line)
{
    if(interactiveAssertion){
        MemoryWriter text;
        text << "in " << function << "():\n";
        text << "\n    " << message;
        if(str){
            text << "\n    ( " << str << " )";
        }
        text << "\n\n";
        text << fileName << ": line " << line << "\n";
        int result = MessageBoxA(0, text.c_str(), "Debug Assertion Triggered", MB_ICONERROR | MB_ABORTRETRYIGNORE);
        switch(result){
        case IDRETRY: return 0;
        case IDIGNORE: return 1;
        case IDABORT: return 2;
        }
        return 0;
    }
    else{
        MemoryWriter text;
        text << "Assertion: " << fileName << ": line " << line << ", " << function << "(): " << message;
        if(str)
            text << " ( " << str << " )";
        fwrite(text.c_str(), text.size(), 1, stderr);
        return 0; // Retry
    }
}

#endif
#endif

}