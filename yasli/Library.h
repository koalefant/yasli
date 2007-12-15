#pragma once

#include "yasli/API.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

#include "yasli/LibraryReference.h"
#include "utils/Content.h"
#include "utils/Errors.h"


class Archive;
class LibraryElementBase;

// ---------------------------------------------------------------------------
class LibraryBase;
typedef LibraryBase&(*LibraryInstanceFunc)(void);
class LibraryDescription;
class YASLI_API LibraryBase : public ContentUser{
public:
    LibraryBase();

    virtual int addBase(const char* name);

    virtual size_t size() const{ ASSERT(0); return 0; }
    virtual LibraryElementBase& elementByIndex(int index){ ASSERT(0); return *(LibraryElementBase*)(0); }
    virtual int findIndexByName(const char* name) const{ ASSERT(0); return -1; }
    
    virtual TypeID libraryTypeID() const{ ASSERT(0); return TypeID(); }
    virtual void serialize(Archive& ar) {}
    virtual void serializePreload(Archive& ar) {}

    void preloadContent();
    void loadContent();
    void saveContent();
    void unloadContent();

    const char* name() const;
    const char* contentGroup() const;

    void setDesc(const LibraryDescription* desc);
    //const StringList& libraryStringList(const LibraryType* dummy);
    const StringList& stringList();
protected:
    void updateStringList();

    const LibraryDescription* desc_;
    StringList stringList_;
};

// ---------------------------------------------------------------------------

class LibraryElementBase;

template<class T, class ReferenceType>
class Library : public LibraryBase{
public:
    Library()
    : loaded_(false)
    {
    }

    typedef std::vector<T> Elements;

    // virtuals:
    TypeID elementTypeID() const{ return TypeID::get<T>(); }
    TypeID libraryTypeID() const{ return TypeID::get<Library>(); };
    T& objectByIndex(int index){
        return elements_.at(index);
    }
    std::size_t size() const{ return elements_.size(); }

    void serializePreload(Archive& ar)
    {
        ASSERT(ar.isInput());
        ASSERT(!loaded_);
        std::vector<std::string> contents;
        ar(contents, "contents");
        elements_.resize(contents.size());
        for(int i = 0; i < int(contents.size()); ++i){
            elements_[i].setName(contents[i].c_str());
            //elements_[i].setIndex(i);
        }
    }
    void serialize(Archive& ar){
        if(!loaded_ && !elements_.empty() && ar.isInput()){
            Elements elements = elements_;
            ar(elements, "elements");
            typename Elements::iterator it;
            for(it = elements.begin(); it != elements.end(); ++it){
                const char* name = it->name();
                int index = findIndexByName(name);
                if(index == -1)
                    elements_.push_back(*it);
                else{
                    std::cout << "Library: Assigning " << index << " (\""
                              << name << "\")" << std::endl;
                    elements_[index] = *it;
                }
            }
            loaded_ = true;
        }
        else{
            std::vector<std::string> contents;
            typename Elements::iterator it;
            for(it = elements_.begin(); it != elements_.end(); ++it){
                const char* name = it->name();
                contents.push_back(name);
            }
            ar(contents, "contents");
            ar(elements_, "elements");
        }
    }
    // ^^^
    T& elementByIndex(int index) 
    {
        ASSERT(index >= 0);
        ASSERT(std::size_t(index) < elements_.size());
        return elements_[index];
    }
    int findIndexByName(const char* name) const{
        typename Elements::const_iterator it;
        int index = 0;
        for(it = elements_.begin(); it != elements_.end(); ++it){
            const LibraryElementBase& element = *it;
            if(strcmp(element.name(), name) == 0)
                return index;
            ++index;
        }
        return -1;
    }

    int addBase(const char* name){
        add(name);
        return elements_.size() - 1;
    }
    ReferenceType add(const char* name){
        T element(name);
        elements_.push_back(element);
        return ReferenceType(elements_.size() - 1);
    }
    void remove(const char* name){
        int elementIndex = findIndexByName(name);
        elements_.erase(elements_.begin() + elementIndex);
    }
protected:
    //LibraryElementBase* createElement() const{ return new T; }

    Elements elements_;
    bool loaded_;
};

// --------------------------------------------------------------------------- 


class LibraryDescription;
class YASLI_API LibraryManager : public ContentUser{
public:
    void serialize(Archive& ar);

    void registerContentGroups();

    static LibraryManager& the();
    LibraryBase* libraryByName(const char* libraryName) const;
protected:
    const LibraryDescription* descByName(const char* libraryName) const;
    const LibraryDescription* descByInstanceFunc(LibraryInstanceFunc func) const;
    void registerLibrary(const LibraryDescription* description);

    typedef std::map<std::string, const LibraryDescription*> Libraries;
    typedef std::map<std::string, std::string> GroupLocations;
    GroupLocations groupLocations_;
    Libraries libraries_;
    friend class LibraryDescription;
    friend class LibraryBase;
};

class YASLI_API LibraryDescription{
public:
    template<class LibraryType>
    LibraryDescription(LibraryType&(*instanceFunc)(void), const char* name, const char* filename, const char* group)
    : name_(name)
    , group_(group)
    , fileName_(filename)
    , instanceFunc_((LibraryInstanceFunc)instanceFunc)
    {
        LibraryManager::the().registerLibrary(this);
    }
    const char* name() const{ return name_; }
    const char* group() const{ return group_; }
    const char* fileName() const{ return fileName_; }
    LibraryBase& instance() const{ return instanceFunc_(); }
    LibraryInstanceFunc instanceFunc() const{ return instanceFunc_; }
protected:
    const char* name_;
    const char* group_;
    const char* fileName_;
    LibraryInstanceFunc instanceFunc_;
};

#define REGISTER_LIBRARY(libraryType, libName, filename, group) \
LibraryDescription libraryType##_description(&libraryType::the, libName, filename, group); \
int libraryFindIndexByName(libraryType* dummy, const char* name){ \
    return libraryType::the().findIndexByName(name); \
} \
LibraryElementBase* libraryElementByIndex(libraryType* dummy, int index){ \
    return &libraryType::the().elementByIndex(index); \
} \
const StringList& libraryStringList(const libraryType* dummy){ return libraryType::the().stringList(); } \
const char* libraryName(const libraryType* dummy){ return libraryType::the().name(); } 
