#include "StdAfx.h"
#include "yasli/Archive.h"
#include "yasli/FileSelector.h"

void FileSelector::serialize(Archive& ar)
{
	ar(fileName_, "");
}
