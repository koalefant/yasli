#include "StdAfx.h"
#include <iostream>
#include <algorithm>
#include "utils/Files.h"
#include "utils/Content.h"
#include "utils/Files.h"
#ifdef WIN32
# include <direct.h>
#else
# include <unistd.h>
#endif

ContentManager* content = 0;

std::string ContentManager::getPath(const char* groupName, const char* folders, const char* elementName) const
{
    std::string result(getPath(groupName));
    result += Files::pathSeparator();
    result += folders;
    result += Files::pathSeparator();
    result += elementName;
    if(Files::pathSeparator()[0] != '/')
        std::replace(result.begin(), result.end(), '/', Files::pathSeparator()[0]);
    return  result;
}

std::string ContentManager::getPath(const char* groupName, const char* elementName)
{
    std::string result(getPath(groupName));
    result += Files::pathSeparator();
    result += elementName;
    if(Files::pathSeparator()[0] != '/')
        std::replace(result.begin(), result.end(), '/', Files::pathSeparator()[0]);
    return result;
}

std::string ContentManager::getPath(const char* groupName) const
{
    GroupDirectories::const_iterator it = groupDirectories_.find(groupName);
    ASSERT(it != groupDirectories_.end() && "ContentManager: No such group registered");
    if(it != groupDirectories_.end())
        return it->second;
    else
        return "";
}

void ContentManager::registerUser(const char* contentGroup, ContentUser* user)
{
    ContentGroupUsers& group = contentGroups_[contentGroup];
    ContentGroupUsers::iterator it = std::find(group.begin(), group.end(), user);
    ASSERT(it == group.end() && "ContentUser already registered within same group.");
    if(it == group.end())
        group.push_back(user);
}

void ContentManager::initGroup(const char* groupName, const char* directoryPath)
{
    char cwd[500 + 1];
#ifdef WIN32
    _getcwd(cwd, 500);
#else
    getcwd(cwd, 500);
#endif
    groupDirectories_[groupName] = std::string(cwd) + Files::pathSeparator() + directoryPath;

    ContentGroups::iterator git = contentGroups_.find(groupName);
    if(git == contentGroups_.end()){
        std::cout << "WARNING: ContentUser::initGroup(): No users registered for group '"
                  << groupName << "'" << std::endl;
        return;
    }

    ContentGroupUsers& users = git->second;
    ContentGroupUsers::iterator it;
    for(it = users.begin(); it != users.end(); ++it){
        ContentUser* user = *it;
        user->preloadContent();
    }
    for(it = users.begin(); it != users.end(); ++it){
        ContentUser* user = *it;
        user->loadContent();
    }
}

void ContentManager::saveGroup(const char* groupName)
{
    ContentGroups::iterator git = contentGroups_.find(groupName);
    if(git == contentGroups_.end()){
        std::cout << "WARNING: ContentUser::initGroup(): No such content group '"
                  << groupName << "'" << std::endl;
        return;
    }

    ContentGroupUsers& users = git->second;
    ContentGroupUsers::iterator it;
    for(it = users.begin(); it != users.end(); ++it){
        ContentUser* user = *it;
        user->saveContent();
    }
}

void ContentManager::unloadAll()
{
    ContentGroups::iterator git;
    for(git = contentGroups_.begin(); git != contentGroups_.end(); ++git){
        ContentGroupUsers& users = git->second;
        ContentGroupUsers::iterator it;
        for(it = users.begin(); it != users.end(); ++it){
            ContentUser* user = *it;
            user->unloadContent();
        }
    }
}
