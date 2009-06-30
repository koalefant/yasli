#include "StdAfx.h"
#include "Colors.h"
#include "yasli/Archive.h"

namespace yasli{

void Color4f::serialize(Archive& ar)
{
    ar(r, "", "&R");
    ar(g, "", "&G");
    ar(b, "", "&B");
    ar(a, "", "&A");
}

void Color3c::serialize(Archive& ar)
{
    ar(r, "", "&R");
    ar(g, "", "&G");
    ar(b, "", "&B");
}

void Color4c::serialize(Archive& ar)
{
    ar(r, "", "&R");
    ar(g, "", "&G");
    ar(b, "", "&B");
    ar(a, "", "&A");
}

}
