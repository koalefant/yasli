#include "StdAfx.h"
#ifdef WIN32
# include <windows.h>
#else
# include <sys/stat.h>
#endif
#include "utils/Files.h"

#include <algorithm>
#include <cstdio>

namespace Files{

bool exists(const char* fileName)
{
    if(std::FILE* file = fopen(fileName, "rb")){
        fclose(file);
        return true;
    }
    else
        return false;
}

bool createDirectory(const char* path)
{
#ifdef WIN32
    return true;
#else
    mode_t mask = umask(0);
    umask(mask);

    int len = strlen(path);
    char* pathCopy = (char*)alloca(len + 1);
    memcpy(pathCopy, path, len + 1);

    char* p = pathCopy;
    while(*p != '\0'){
        if(*p == '/'){
            *p = '\0';
            if(mkdir(path, mask) != 0)
                return false;
            *p = '/';
        }
    }

    return mkdir(pathCopy, mask) == 0;
#endif
}

std::string extractFileBase(const char* path)
{
    const char* fileNameStart = path;
    const char* end = path + strlen(path);
    const char* p = std::find(path, end, pathSeparator()[0]);
    if(p != end){
        fileNameStart = p + 1;
    }
    p = end;
    while(--p != fileNameStart){
        if(*p =='.') 
            break;
    }
    if(p == fileNameStart)
        p = end;
	return std::string(fileNameStart, p);
}


bool createDirectoryForFile(const char* path)
{
    size_t len = strlen(path);
    const char* p = path + len;
    while(--p != path){
        if(*p == pathSeparator()[0]){
            std::string directoryPath(path, p);
            return createDirectory(directoryPath.c_str());
        }
    }
    return true;
}

bool copy(const char* sourceFile, const char* destinationFile)
{
#ifdef WIN32
	return CopyFile(sourceFile, destinationFile, FALSE) != FALSE;
#else
    return system((std::string("cp \"") + sourceFile + "\" \"" + destinationFile + "\"").c_str()) == 0;
#endif
}

}
