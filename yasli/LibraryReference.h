#pragma once

#include "utils/StringList.h"
#include "utils/SafeCast.h"
#include "yasli/API.h"

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
    operator bool() const{
        return index_ != -1;
    }
protected:
    int index_;
	friend class LibrarySelector;
};

class LibraryReferenceBase;
class LibrarySelector{
public:
	LibrarySelector()
	: referencePtr_(0)
	, libraryName_("")
	{
	}
	LibrarySelector(LibraryReferenceBase* ref, const char* libraryName)
	: referencePtr_(ref)
	, reference_(*ref)
	, libraryName_(libraryName)
	{

	}
	~LibrarySelector(){
        if(referencePtr_)
			*referencePtr_ = reference_;
	}
	int index() const{ return reference_.index_; }
	void setIndex(int index){ reference_.index_ = index; }
	void serialize(Archive& ar);
	const char* libraryName() const{ return libraryName_; }
	const char* elementName() const;
protected:
	LibraryReferenceBase reference_;
	LibraryReferenceBase* referencePtr_;
	const char* libraryName_;
};

template<class ElementType, class LibraryType>
class LibraryReference : public LibraryReferenceBase{
public:
    LibraryReference(const char* elementName = "")
    {
        int libraryFindIndexByName(LibraryType* dummy, const char* elementName);

        if(elementName[0] == 0)
            index_ = -1;
        else{
            index_ = libraryFindIndexByName((LibraryType*)(0), elementName);
            //ASSERT(index_ != -1);
        }
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
    ElementType* get(){
        LibraryElementBase* libraryElementByIndex(LibraryType* dummy, int index);
        if(index_ >= 0)
            return (ElementType*)(libraryElementByIndex((LibraryType*)0, index_));
        else
            return 0;
    }
    ElementType* get() const{
        LibraryElementBase* libraryElementByIndex(LibraryType* dummy, int index);
        if(index_ >= 0)
            return (ElementType*)(libraryElementByIndex((LibraryType*)0, index_));
        else
            return 0;
    }
    ElementType& operator*(){
        ElementType* element = get();
        ASSERT(element);
        return *element;
    }
    ElementType& operator*() const{
        ElementType* element = get();
        ASSERT(element);
        return *element;
    }
    ElementType* operator->(){
        ElementType* element = get();
        ASSERT(element);
        return element;
    }
    ElementType* operator->() const{
        ElementType* element = get();
        ASSERT(element);
        return element;
    }

    // non-template member will fail to compile without class Archive declaration
    template<class Archive>
    void serialize(Archive& ar){
        int libraryFindIndexByName(LibraryType* dummy, const char* elementName);
        const StringList& libraryStringList(const LibraryType* dummy);
        const char* libraryName(const LibraryType* dummy);
		
		if(ar.isEdit()){
			ElementType* element = get();
			const char* name = libraryName((LibraryType*)(0));
			LibrarySelector selector(this, name);
			ar(selector, "");
		}
		else{
			ElementType* element = get();
			std::string value;
			if(ar.isOutput() && element)
				value = ((LibraryElementBase*)(element))->name();

			ar(value, "");

			if(ar.isInput())
				index_ = libraryFindIndexByName((LibraryType*)(0), value.c_str());
		}
    }
    bool operator<(const LibraryReference& rhs) const{
        return index_ < rhs.index_;
    }
    bool operator>(const LibraryReference& rhs) const{
        return index_ > rhs.index_;
    }
protected:
    LibraryReference(int index){
        index_ = index;
    }
    template<class T, class ReferenceType>
    friend class Library;
};
