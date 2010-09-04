#pragma once

#include "ww/API.h"
#include <string>

namespace ww{

class Widget;
class WW_API FileDialog{
public:
	FileDialog(ww::Widget* owner, bool save, const char** masks, const char* startDirectory = 0, const char* startFileName = 0, bool allow_multiselect = false);
	bool showModal();
	const char* fileName() const{ return fileName_.c_str(); }
	const std::vector<std::string>& fileNames() const { return fileNames_; }

	void serialize(Archive& ar);
protected:
	HWND ownerWnd_;
	std::string startDirectory_;
	std::string startFileName_;
	std::string fileName_;
	std::vector<std::string> fileNames_;
	std::vector<std::string> masks_;
	bool save_;
	bool multiselect_;
};

}

