#pragma once
#include "yasli/Content.h"

namespace yasli{
class Archive;
class LibraryElementBase;

// ---------------------------------------------------------------------------
class LibraryBase;
typedef LibraryBase&(*LibraryInstanceFunc)(void);
class LibraryDescription;

class YASLI_API LibraryBase : public RefCounter, public ContentUser{
public:
    LibraryBase();

    virtual int addBase(const char* name);
    virtual size_t size() const{ ASSERT(0); return 0; }
    virtual LibraryElementBase& elementByIndex(int index){ ASSERT(0); return *(LibraryElementBase*)(0); }
    virtual int findIndexByName(const char* name) const{ ASSERT(0); return -1; }
	virtual void remove(const char* name) { ASSERT(0); }
    
    virtual TypeID libraryTypeID() const{ ASSERT(0); return TypeID(); }
    virtual void serialize(Archive& ar) {}
    virtual void serializePreload(Archive& ar) {}
	virtual void fixNames() { updateStringList(); }

    // from ContentUser:
    void preloadContent();
    void loadContent();
    void saveContent();
    void unloadContent();
    // ^^^

    const char* name() const;
    const char* contentGroup() const;

    void setDesc(const LibraryDescription* desc);
    const StringList& stringList();
protected:
    void updateStringList();

    const LibraryDescription* desc_;
    StringList stringList_;
};

// ---------------------------------------------------------------------------
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
    LibraryDescription(LibraryType&(*instanceFunc)(void), const char* name, const char* filename, const char* group, bool dummy = false)
    : dummy_(dummy)
	, name_(name)
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
	bool dummy_;
    const char* name_;
    const char* group_;
    const char* fileName_;
    LibraryInstanceFunc instanceFunc_;
};
}

#define REGISTER_LIBRARY(libraryType, libName, filename, group) \
	LibraryDescription libraryType##_description(&libraryType::the, libName, filename, group); 

