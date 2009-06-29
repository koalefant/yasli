#pragma once
#include "LibraryReference.h"

namespace yasli{

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
	LibraryReferenceBase* referencePtr_;
	LibraryReferenceBase reference_;
	const char* libraryName_;
};

}
