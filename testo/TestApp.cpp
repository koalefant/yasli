#include "Testo.h"

void redirectStdOut(const char* _file){/*{{{*/

    FILE* new_file = freopen(_file, "w", stdout);
    if(new_file == 0){
#if !defined (stdout)
        stdout = fopen (_file, "w");
#else
        new_file = fopen (_file, "w");
#endif
        if(new_file)
			*stdout = *new_file;
    }
}/*}}}*/

#ifdef WIN32
#include <windows.h>
#include <stdlib.h>
INT WINAPI WinMain( HINSTANCE inst, HINSTANCE prevInst, LPSTR commandLine, INT)
{
    int argc = __argc;
    char** argv = __argv;
	//redirectStdOut((std::string(argv[0]) + ".log").c_str());
#else
int main(int argc, char* argv[])
{
#endif
    return Testo::Manager::the().invokeAll();
}
