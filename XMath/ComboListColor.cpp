#include "StdAfx.h"
#include "XMath/Colors.h"
#include "ComboListColor.h"

void ComboListColor::serialize(yasli::Archive& ar)
{
	ar.serialize(index_, "index", 0);
}

