#pragma once
#include <string>

namespace yasli{
	class Archive;
}

namespace ww{
	class FileSelector;
}

bool serialize(yasli::Archive& ar, ww::FileSelector& selector, const char* name, const char* label);

namespace ww{

using std::string;

class FileSelector{
public:
	struct Options{
		Options(const char* _filter, bool _save = false, const char* _rootDirectory = "")
		: filter(_filter)
		, save(_save)
        , rootDirectory(_rootDirectory)
		{
		}
		string filter;
		string rootDirectory;
		bool save;
	};

	FileSelector()
	: fileNamePtr_(0)
	, options_(0)
	{
	}
	
	FileSelector(const FileSelector& _original)
	: fileNamePtr_(0)
	, fileName_(_original.fileName_)
	, options_(_original.options_)
	{
	}

	FileSelector(std::string& fileName, const Options& options)
	: fileNamePtr_(&fileName)
	, fileName_(fileName)
	, options_(&options)
	{
	}

	~FileSelector(){
		if(fileNamePtr_)
			*fileNamePtr_ = fileName_;
	}

	FileSelector& operator=(const FileSelector& rhs){
		fileName_ = rhs.fileName_;
		return *this;
	}
	FileSelector& operator=(const char* rhs){
		fileName_ = rhs;
		return *this;
	}
	const char* c_str() const{ return fileName_.c_str(); }
	const Options* options(){ return options_; }

	void serialize(yasli::Archive& ar);
protected:
	std::string* fileNamePtr_;
	std::string fileName_;
	const Options* options_;
	friend bool ::serialize(yasli::Archive& ar, ww::FileSelector& selector, const char* name, const char* label);
};

}

