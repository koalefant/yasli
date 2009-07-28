#pragma once
#include "yasli/Pointers.h"
#include <time.h>

namespace Files{
    using std::string;
#ifdef WIN32
	static const char* PATH_SEPARATOR = "\\";
#else
	static const char* PATH_SEPARATOR = "/";
#endif
    inline const char* pathSeparator(){ return PATH_SEPARATOR; }

	string extractFileBase(const char* path);
	string extractFilePath(const char* path);
	inline const char* extractFileName(const char* path){
		size_t len = strlen(path);
		const char* p = path + len - 1;
		while(p != path){
			--p;
			if(*p == PATH_SEPARATOR[0])
				return p + 1;
		}
		return path;
	}
    string relativePath(const char* path, const char* toDirectory);


    bool exists(const char* fileName);
	bool isDirectory(const char* fileName);

    bool createDirectory(const char* path);
	bool createDirectoryForFile(const char* path);
	bool remove(const char* path);

    bool copy(const char* sourceFile, const char* destinationFile);

	string setExtention(const char* file_name, const char* extention);

#ifdef WIN32
	typedef ::__time64_t time_t;
#else
	typedef ::time_t time_t;
#endif

	time_t getModifyTime(const char* path);

	class iteratorImpl;

	class DirectoryEntry{
	public:
		const char* name() const{ return name_.c_str(); }
		bool isFile() const{ return !isDirectory_; }
		bool isDirectory() const{ return isDirectory_; }
	protected:
		string name_;
		bool isDirectory_;
		friend class iteratorImpl;
	};


	class iterator{
	public:
		iterator(const char* path = 0);
		~iterator();
		iterator(const iterator& original);
		iterator& operator=(const iterator& rhs);
		
		iterator& operator++();
		bool operator==(const iterator& rhs);
		bool operator!=(const iterator& rhs);
		operator bool() const;

		DirectoryEntry& operator*();
		DirectoryEntry* operator->();
	protected:
		void set(iteratorImpl* impl);

		iteratorImpl* impl_;
	};
}
