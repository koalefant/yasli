#pragma once

namespace Files{
    inline const char* pathSeparator(){
#ifdef WIN32
        return "\\";
#else
        return "/";
#endif
    }

	std::string extractFileBase(const char* path);

    bool exists(const char* fileName);

    bool createDirectory(const char* path);

    bool copy(const char* sourceFile, const char* destinationFile);
}
