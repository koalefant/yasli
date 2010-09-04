#include "stdafx.h"
#include "Rectf.h"
#include "yasli/Archive.h"
using namespace yasli;

void Rectf::serialize(Archive& ar)
{
  ar( min_.x, "", "&minX" );
  ar( min_.y, "", "&minY" );
  ar( max_.x, "", "&maxX" );
  ar( max_.y, "", "&maxY" );
}
