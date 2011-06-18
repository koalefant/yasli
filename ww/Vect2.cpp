#include "StdAfx.h"
#include "Vect2.h"
#include "yasli/Archive.h"

namespace ww {

const Vect2 Vect2::ZERO;

void Vect2::serialize(yasli::Archive& ar)
{
	ar(x, "", "^x");
	ar(y, "", "^y");
}

}
