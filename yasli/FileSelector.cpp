#include "StdAfx.h"
#include "yasli/Archive.h"
#include "yasli/FileSelector.h"

namespace yasli{

void FileSelector::serialize(Archive& ar)
{
	ar(fileName_, "fileName", "File Name");
}

}

bool serialize(yasli::Archive& ar, yasli::FileSelector& selector, const char* name, const char* label)
{
    if(ar.isEdit())
        return ar(yasli::Serializer(selector), name, label);
    else
        return ar(selector.fileName_, name, label);
}


