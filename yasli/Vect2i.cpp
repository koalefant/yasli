#include "stdafx.h"
#include "Vect2i.h"
#include "Vect2f.h"
#include "yasli/Archive.h"
#include "yasli/round.h"

namespace yasli{

Vect2i::Vect2i(const Vect2f& vect)
: x(round(vect.x))
, y(round(vect.y))
{

}

void Vect2i::serialize( Archive& ar )
{
  ar( x, "", "x" );
  ar( y, "", "y" );
}

}