#pragma once

#include "yasli/StringList.h"
#include "yasli/SafeCast.h"
#include "yasli/API.h"

namespace yasli{

class Archive;

class YASLI_API LibraryElementBase{
public:
    LibraryElementBase(const char* name = "");
    virtual ~LibraryElementBase() {}

    // virtuals:
    virtual void serialize(Archive& ar);
    // ^^^
    const char* name() const{ return name_.c_str(); }
    void setName(const char* name){ name_ = name; }
protected:
    std::string name_;
};

class LibraryReferenceBase{
public:
    operator bool() const{ return index_ != -1; }
protected:
    int index_;
	friend class LibrarySelector;
};

template<class ElementType, class LibraryType>
class LibraryReference : public LibraryReferenceBase{
public:
    LibraryReference(const char* elementName = "")
    {
        if(elementName[0] == 0)
            index_ = -1;
        else
            index_ = findIndexByName(elementName);
    }
    bool operator==(const LibraryReference& rhs) const{ return index_ == rhs.index_; }
    bool operator!=(const LibraryReference& rhs) const{ return index_ != rhs.index_; }
    const char* c_str() const{
        LibraryElementBase* element = get();
        if(element)
            return element->name();
        else
            return "";
    }
    ElementType* get() const{
        if(index_ >= 0)
            return (ElementType*)elementByIndex(index_);
        else
            return 0;
    }
	ElementType* getNotNull() const{
		ElementType* element = get();
		ASSERT(element);
		return element;
	}

    ElementType& operator*(){ return *getNotNull(); }
    ElementType& operator*() const{ return *getNotNull(); }
    ElementType* operator->(){ return getNotNull(); }
    ElementType* operator->() const{ return getNotNull(); }

    bool operator<(const LibraryReference& rhs) const{
        return index_ < rhs.index_;
    }
    bool operator>(const LibraryReference& rhs) const{
        return index_ > rhs.index_;
    }

	// moved into LibraryReferenceImpl.h
    void serialize(Archive& ar);
	int findIndexByName(const char* elementName) const;
	LibraryElementBase* elementByIndex(int index) const;
	const char* libraryName() const;
	static bool instantiate();
	// ^^^
protected:
    LibraryReference(int index){
        index_ = index;
    }
    template<class T, class ReferenceType>
    friend class Library;
};

}