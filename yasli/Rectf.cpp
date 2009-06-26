#include "stdafx.h"
#include "Rectf.h"
#include "yasli/Archive.h"

void Rectf::serialize(Archive& ar)
{
  ar( min.x, "" );
  ar( min.y, "" );
  ar( max.x, "" );
  ar( max.y, "" );
}
