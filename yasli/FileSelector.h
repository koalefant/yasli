#pragma once
#include <string>

class Archive;
class FileSelector{
public:
	struct Options{
		Options(const char* _filter)
		: filter(_filter)
		{
		}
		std::string filter;
	};

	FileSelector()
	: fileNamePtr_(0)
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
};
