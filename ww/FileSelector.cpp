#include "StdAfx.h"
#include "yasli/Archive.h"
#include "ww/FileSelector.h"

namespace ww{

void FileSelector::serialize(yasli::Archive& ar)
{
	ar(fileName_, "fileName", "File Name");
}

}

bool serialize(yasli::Archive& ar, ww::FileSelector& selector, const char* name, const char* label)
{
    if(ar.isEdit())
        return ar(yasli::Serializer(selector), name, label);
    else
        return ar(selector.fileName_, name, label);
}


