#pragma once
#include <map>
#include <vector>
#include "yasli/ConstString.h"

class ContentUser{
public:
    virtual ~ContentUser() {}
    virtual void saveContent() {}
    virtual void preloadContent() {}
    virtual void loadContent() {}
    virtual void unloadContent() {}
};

class ContentManager{
public:
    void initGroup(const char* groupName, const char* directoryPath);
    void saveGroup(const char* groupName);
    void unloadAll();

    std::string getPath(const char* groupName, const char* folders, const char* elementName) const;
    std::string getPath(const char* groupName, const char* elementName);
    std::string getPath(const char* groupName) const;

    void registerUser(const char* group, ContentUser* user);
protected:
    typedef std::vector<ContentUser*> ContentGroupUsers;
    typedef std::map<ConstString, ContentGroupUsers> ContentGroups;
    typedef std::map<ConstString, std::string> GroupDirectories;
    ContentGroups contentGroups_;
    GroupDirectories groupDirectories_;
};

extern ContentManager* content;
