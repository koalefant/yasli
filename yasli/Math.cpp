#include "StdAfx.h"
#include "utils/Math.h"
#include "yasli/Archive.h"

void Vector2i::serialize(Archive& ar)
{
    ar(x);
    ar(y);
}
