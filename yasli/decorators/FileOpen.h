#pragma once

#include "yasli/Config.h"

namespace yasli {

class Archive;

struct FileOpen
{
	string* pathPointer;
	string filter;
	string relativeToFolder;
	int flags;

	enum {
		STRIP_EXTENSION = 1
	};

	// filter is defined in the following format:
	// "All Images (*.bmp *.jpg *.tga);; Bitmap (*.bmp);; Targa (*.tga)"
	FileOpen(string& path, const char* filter, const char* relativeToFolder = "", int flags = 0)
	: pathPointer(&path)
	, filter(filter)
	, relativeToFolder(relativeToFolder)
	, flags(flags)
	{
	}

    FileOpen() : pathPointer(nullptr), flags(0) { }


	~FileOpen()
	{
	}

	FileOpen& operator=(const FileOpen&) = delete;
};

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, FileOpen& value, const char* name, const char* label);

}

#if YASLI_INLINE_IMPLEMENTATION
#include "yasli/decorators/FileOpen.cpp"
#endif
