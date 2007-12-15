#include "StdAfx.h"
#include <iostream>

#include "utils/Files.h"

#include "yasli/Library.h"

#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"
#include "yasli/PointersImpl.h"

#include "yasli/TextIArchive.h"
#include "yasli/TextOArchive.h"

// ---------------------------------------------------------------------------

LibraryBase::LibraryBase()
{
}

int LibraryBase::addBase(const char* name)
{
    ASSERT(0);
    return -1;
}

void LibraryBase::setDesc(const LibraryDescription* desc)
{
    desc_ = desc;
}

void LibraryBase::preloadContent()
{
    std::string filename = content->getPath(contentGroup(), name());
    //std::cout << "Loading library: " << filename << std::endl;
    TextIArchive ia;

    try{
        ia.open(filename.c_str());
        serializePreload(ia);
    }
    catch(ErrorGeneric& error){
        std::cout << "Error loading library from " << filename << ":" << error.what() << std::endl;
    }
}

void LibraryBase::loadContent()
{
    std::string filename = content->getPath(contentGroup(), name());
    std::cout << "Loading library: " << filename << std::endl;
    TextIArchive ia;

    try{
        ia.open(filename.c_str());
        serialize(ia);
    }
    catch(ErrorGeneric& error){
        std::cout << "Error loading library from " << filename << ":" << error.what() << std::endl;
    }
}

void LibraryBase::unloadContent()
{
    std::cout << "Unloading library: " << name() << std::endl;
    saveContent();
}

const char* LibraryBase::contentGroup() const
{
    return desc_->group();
}

void LibraryBase::saveContent()
{
    std::string filename = content->getPath(contentGroup(), name());

    ASSERT(!filename.empty());
    TextOArchive oa;

    std::cout << "Saving library: " << filename << std::endl;

    try{
        oa.open(filename.c_str());
        serialize(oa);
    }
    catch(ErrorGeneric& error){
        std::cout << "Error saving library " << filename << ":" << error.what() << std::endl;
    }
}

const char* LibraryBase::name() const
{
	ASSERT(desc_);
    return desc_->name();
}

const StringList& LibraryBase::stringList()
{
    updateStringList();
    return stringList_;
}

void LibraryBase::updateStringList()
{
    int size = this->size();
    stringList_.clear();
    //stringList_.push_back("");
    for(int i = 0; i < size; ++i){
        LibraryElementBase* element = &elementByIndex(i);
        ASSERT(element);
        stringList_.push_back(element->name());
    }
}

// ---------------------------------------------------------------------------

LibraryElementBase::LibraryElementBase(const char* name)
: name_(name)
{
}


void LibraryElementBase::serialize(Archive& ar)
{
    ar(name_, "");
}

// --------------------------------------------------------------------------- 

//LibraryManager::LibraryDescription::LibraryDescription(LibraryInstanceFunc instanceFunc, const char* name, const char* filename, const char* group)

void LibraryManager::registerLibrary(const LibraryDescription* description)
{
    libraries_[description->name()] = description;
}

YASLI_API LibraryManager& LibraryManager::the()
{
    static LibraryManager librariesManager;
    return librariesManager;
}

void LibraryManager::serialize(Archive& ar)
{
    /*
    Libraries::iterator it;
    for(it = libraries_.begin(); it != libraries_.end(); ++it){
        const char* name = it->first.c_str();

        LibraryInstanceFunc instance = it->second;
        ASSERT(instance);
        LibraryBase& library = instance();
        ASSERT(name && strlen(name));
        ar(library, name);
    }
    */
    ASSERT(0);
}

void LibraryManager::registerContentGroups()
{
    ASSERT(content && "ContentManager shall be already created");
    Libraries::const_iterator it;
    for(it = libraries_.begin(); it != libraries_.end(); ++it){
        const LibraryDescription* desc = it->second;

        LibraryBase& library = desc->instance();
        library.setDesc(desc);
        content->registerUser(library.contentGroup(), &library);
    }
}


LibraryBase* LibraryManager::libraryByName(const char* libraryName) const
{
    const LibraryDescription* desc = descByName(libraryName);
    if(!desc)
        return 0;
    else
        return &desc->instance();
}

const LibraryDescription* LibraryManager::descByName(const char* libraryName) const
{
    Libraries::const_iterator it;
    for(it = libraries_.begin(); it != libraries_.end(); ++it){
        const LibraryDescription* desc = it->second;
        if(strcmp(desc->name(), libraryName) == 0)
            return desc;
    }
    return 0;
}

const LibraryDescription* LibraryManager::descByInstanceFunc(LibraryInstanceFunc func) const
{
    Libraries::const_iterator it;
    for(it = libraries_.begin(); it != libraries_.end(); ++it){
        const LibraryDescription* desc = it->second;
        if(desc->instanceFunc() == func)
            return desc;
    }
    return 0;
}

// ---------------------------------------------------------------------------

void LibrarySelector::serialize(Archive& ar){
	ASSERT(this);
	ar(reference_.index_, "index");
	std::string libraryName(libraryName_);
	ar(libraryName, "libraryName");
	if(ar.isInput()){
		LibraryBase* library = LibraryManager::the().libraryByName(libraryName.c_str());
		if(library)
			libraryName_ = library->name();
		else
			libraryName_ = "";
	}
}

const char* LibrarySelector::elementName() const
{
	LibraryBase* library = LibraryManager::the().libraryByName(libraryName_);
	if(library && reference_.index_ >= 0 && reference_.index_ < library->size())
		return library->elementByIndex(reference_.index_).name();
	return "";
}
