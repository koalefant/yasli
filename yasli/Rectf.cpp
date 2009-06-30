#include "stdafx.h"
#include "Rectf.h"
#include "yasli/Archive.h"

void Rectf::serialize(Archive& ar)
{
  ar( min.x, "", "&minX" );
  ar( min.y, "", "&minY" );
  ar( max.x, "", "&maxX" );
  ar( max.y, "", "&maxY" );
}
