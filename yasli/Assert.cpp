#include "StdAfx.h"
#include "MemoryWriter.h"

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

#ifndef NDEBUG

int assertionDialog(const char* message, const char* function, const char* fileName, int line)
{
    MemoryWriter text;
    //text << "Debug Assertion Triggered:\n\n";
    text << "in " << function << "():\n";
    text << "\n    " << message;
    text << "\n\n";
    text << fileName << ": line " << line << "\n";
    int result = MessageBox(0, text.c_str(), "Debug Assertion Triggered", MB_ICONERROR | MB_ABORTRETRYIGNORE);
    switch(result){
    case IDRETRY: return 0;
    case IDIGNORE: return 1;
    case IDABORT: return 2;
    }
    return 0;
}

#endif
#endif
