#include "StdAfx.h"
#include "yasli/Archive.h"
#include "yasli/FileSelector.h"

void FileSelector::serialize(Archive& ar)
{
	ar(fileName_, "");
}

bool serialize(Archive& ar, FileSelector& selector, const char* name, const char* label)
{
    if(ar.isEdit())
        return ar(Serializer(selector), name, label);
    else
        return ar(selector.fileName_, name, label);
}
