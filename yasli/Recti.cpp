#include "stdafx.h"
#include "Recti.h"
#include "yasli/Archive.h"

namespace yasli{

void Recti::serialize(Archive& ar)
{
  ar( min.x, "", "&minX" );
  ar( min.y, "", "&minY" );
  ar( max.x, "", "&maxX" );
  ar( max.y, "", "&maxY" );
}

}
