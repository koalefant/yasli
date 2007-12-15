#include "StdAfx.h"
#include "uMath/Colors.h"
#include "yasli/Colors.h"
#include "yasli/Archive.h"

struct Color4cSerializeable : public Color4c{
    void serialize(Archive& ar){
        ar(r);
        ar(g);
        ar(b);
        ar(a);
    }
};


bool serialize(Archive& ar, Color4c& color, const char* name)
{
    return ar(static_cast<Color4cSerializeable&>(color), name);
}
