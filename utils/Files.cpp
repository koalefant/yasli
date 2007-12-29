#include "StdAfx.h"
#ifdef WIN32
# include <windows.h>
#else
# include <sys/stat.h>
#endif
#include "utils/Files.h"

#include <malloc.h>
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

bool isDirectory(const char* fileName)
{
	DWORD attributes = GetFileAttributes(fileName);
	if(attributes == INVALID_FILE_ATTRIBUTES)
		return false;
	return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool remove(const char* fileName)
{
#ifdef WIN32
	return DeleteFile(fileName) != FALSE;
#else
	return false;
#endif
}

bool createDirectory(const char* path)
{
#ifndef WIN32
    mode_t mask = umask(0);
    umask(mask);
#endif

    int len = strlen(path);
    char* pathCopy = (char*)alloca(len + 1);
    memcpy(pathCopy, path, len + 1);

    char* p = pathCopy;
    while(*p != '\0'){
        if(*p == PATH_SEPARATOR[0]){
            *p = '\0';
#ifdef WIN32
			if(!CreateDirectory(path, 0))
				return false;
#else
            if(mkdir(path, mask) != 0)
                return false;
#endif
            *p = PATH_SEPARATOR[0];
        }
		++p;
    }

#ifdef WIN32
	return CreateDirectory(pathCopy, 0) != FALSE;
#else
    return mkdir(pathCopy, mask) == 0;
#endif
}

std::string extractFilePath(const char* path)
{
	size_t len = strlen(path);
	if(len == 0)
		return std::string();

	const char* p = path + len - 1;
	while(p != path){
		if(p[0] == PATH_SEPARATOR[0])
			return std::string(path, p);
		--p;
	}
	return std::string();
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

// ---------------------------------------------------------------------------

#ifdef WIN32
class iteratorImpl : public RefCounter{
public:
	iteratorImpl()
	{
		handle_ = INVALID_HANDLE_VALUE;
	}
	iteratorImpl(const char* path)
	{
		handle_ = FindFirstFile(path, &findData_);
		if(handle_ == INVALID_HANDLE_VALUE)
			return;
		ASSERT(strcmp(findData_.cFileName, ".") == 0);
		if(!FindNextFile(handle_, &findData_)){
			handle_ = INVALID_HANDLE_VALUE;
			return;
		}
		ASSERT(strcmp(findData_.cFileName, "..") == 0);
		if(!FindNextFile(handle_, &findData_)){
			handle_ = INVALID_HANDLE_VALUE;
			return;
		}
		entry_.name_ = findData_.cFileName;
		entry_.isDirectory_ = (findData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	~iteratorImpl(){
		if(handle_ != INVALID_HANDLE_VALUE)
			FindClose(handle_);
	}

	bool compare(const iteratorImpl& rhs){
		return handle_ == rhs.handle_;
	}

	bool next()
	{
		ASSERT(handle_ != INVALID_HANDLE_VALUE && "Incrementing bad Files::iterator");
		if(!FindNextFile(handle_, &findData_)){
			FindClose(handle_);
			handle_ = INVALID_HANDLE_VALUE;
			return false;
		}
		else{
			entry_.name_ = findData_.cFileName;
			entry_.isDirectory_ = (findData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			return true;
		}
	}
	bool isValid(){
		return handle_ != INVALID_HANDLE_VALUE;
	}

	WIN32_FIND_DATA findData_;
	HANDLE handle_;
	DirectoryEntry entry_;

	friend class iterator;
};
#else

#endif

// ---------------------------------------------------------------------------

iterator::iterator(const char* path)
: impl_(0)
{
	if(path){
		set(new iteratorImpl(path));
		if(!impl_->isValid())
			set(0);
	}
}

iterator::iterator(const iterator& original)
: impl_(0)
{
	set(original.impl_);
}

iterator::~iterator()
{
	set(0);
}

void iterator::set(iteratorImpl* impl)
{
	if(impl_ != impl){
		if(impl_ && !impl_->release())
			delete impl_;

		impl_ = impl;
		if(impl_)
			impl_->acquire();
	}
}

iterator& iterator::operator=(const iterator& rhs)
{
	set(rhs.impl_);
	return *this;
}

DirectoryEntry& iterator::operator*()
{
	if(impl_)
		return impl_->entry_;
	else{
		ASSERT(0 && "Accessing invalid Files::iterator");
		return *(DirectoryEntry*)(0);
	}
}

DirectoryEntry* iterator::operator->()
{
	return &operator*();
}

bool iterator::operator!=(const iterator& rhs)
{
	return !operator==(rhs);
}

bool iterator::operator==(const iterator& rhs)
{
	if(!impl_ && !rhs.impl_)
		return true;
	if(impl_ && !rhs.impl_ || !impl_ && rhs.impl_)
		return false;
	return impl_->compare(*rhs.impl_);
}

iterator& iterator::operator++()
{
	if(impl_){
		if(impl_->refCount() > 1)
			impl_ = new iteratorImpl(*impl_);

		if(!impl_->next())
			impl_ = 0;
	}
	else{
		ASSERT(0 && "Incrementing invalid Files::iterator");
	}
	return *this;
}

iterator::operator bool() const
{
	return impl_ != 0;
}

}
