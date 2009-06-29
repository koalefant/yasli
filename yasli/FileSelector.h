#pragma once
#include <string>

namespace yasli{
	class Archive;
	class FileSelector;
}

bool serialize(yasli::Archive& ar, yasli::FileSelector& selector, const char* name, const char* label);

namespace yasli{

class Archive;

class FileSelector{
public:
	struct Options{
		Options(const char* _filter, bool _save = false, const char* _initialDirectory = ".")
		: filter(_filter)
		, save(_save)
		, initialDirectory(_initialDirectory)
		{
		}
		std::string filter;
		std::string initialDirectory;
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

	void serialize(Archive& ar);
protected:
	std::string* fileNamePtr_;
	std::string fileName_;
	const Options* options_;
	friend bool ::serialize(yasli::Archive& ar, yasli::FileSelector& selector, const char* name, const char* label);
};

}

