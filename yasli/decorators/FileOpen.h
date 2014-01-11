#pragma once

#include "yasli/Strings.h"

namespace yasli {

class Archive;

struct FileOpen
{
	string* pathPointer;
	string path;
	string filter;
	string relativeToFolder;

	// filter is defined in the following format:
	// "All Images (*.bmp *.jpg *.tga);; Bitmap (*.bmp);; Targa (*.tga)"
	FileOpen(string& path, const char* filter, const char* relativeToFolder = "")
	: pathPointer(&path)
	, filter(filter)
	, relativeToFolder(relativeToFolder)
	{
		this->path = path;
	}

	FileOpen() : pathPointer(0) { }

	FileOpen& operator=(const FileOpen& rhs)
	{
		path = rhs.path;
		if (rhs.pathPointer) {
			path = rhs.path;
			filter = rhs.filter;
			relativeToFolder = rhs.relativeToFolder;
		}
		return *this;
	}

	~FileOpen()
	{
		if (pathPointer)
			*pathPointer = path;
	}

	void serialize(Archive& ar);
};

bool serialize(Archive& ar, FileOpen& value, const char* name, const char* label);

}
