#include "stdafx.h"
#include "Vect2i.h"
#include "yasli/Archive.h"

void Vect2i::serialize( Archive& ar )
{
  ar( x, "" );
  ar( y, "" );
}
