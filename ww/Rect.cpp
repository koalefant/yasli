#include "StdAfx.h"
#include "Rect.h"
#include "yasli/Archive.h"

namespace ww
{

void Rect::serialize(yasli::Archive& ar)
{
  ar( min_.x, "", "&minX" );
  ar( min_.y, "", "&minY" );
  ar( max_.x, "", "&maxX" );
  ar( max_.y, "", "&maxY" );
}

}
