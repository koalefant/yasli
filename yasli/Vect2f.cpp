#include "stdafx.h"
#include "Vect2f.h"
#include "Vect2i.h"
#include "yasli/Archive.h"

namespace yasli{

Vect2f::Vect2f(const Vect2i &value)
: x( (float)value.x )
, y( (float)value.y )
{
}

void Vect2f::serialize( Archive& ar )
{
  ar( x, "", "x" );
  ar( y, "", "y" );
}

}