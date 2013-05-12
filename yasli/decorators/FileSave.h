#pragma once
#include "yasli/Archive.h"

namespace yasli {

struct FileSave
{
	string* pathPointer;
	string path;
	string filter;
	string relativeToFolder;

	// filter is defined in the following format:
	// "All Images (*.bmp *.jpg *.tga);; Bitmap (*.bmp);; Targa (*.tga)"
	FileSave(string& path, const char* filter, const char* relativeToFolder = "")
	: pathPointer(&path)
	, filter(filter)
	, relativeToFolder(relativeToFolder)
	{
		this->path = path;
	}

	FileSave() : pathPointer(0) { }

	FileSave& operator=(const FileSave& rhs)
	{
		path = rhs.path;
		if (rhs.pathPointer) {
			path = rhs.path;
			filter = rhs.filter;
			relativeToFolder = rhs.relativeToFolder;
		}
		return *this;
	}

	~FileSave()
	{
		if (pathPointer)
			*pathPointer = path;
	}

	void serialize(Archive& ar);
};

}

bool serialize(yasli::Archive& ar, yasli::FileSave& value, const char* name, const char* label);
