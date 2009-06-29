#include "stdafx.h"
#include "Recti.h"
#include "yasli/Archive.h"

namespace yasli{

void Recti::serialize(Archive& ar)
{
  ar( min.x, "" );
  ar( min.y, "" );
  ar( max.x, "" );
  ar( max.y, "" );
}

}