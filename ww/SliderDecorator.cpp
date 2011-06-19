#include "stdafx.h"
#include "SliderDecorator.h"
#include "yasli/Archive.h"

using namespace yasli;

namespace ww {

void SliderDecoratorf::clip()
{
	value_ = min(max(value_, minValue_), maxValue_);
}

void SliderDecoratorf::serialize(Archive& ar)
{
  ar.serialize(value_, "value", 0);
  ar.serialize(minValue_, "min", 0);
  ar.serialize(maxValue_, "max", 0);
  ar.serialize(step_, "step", 0);
}


void SliderDecoratori::clip()
{
	value_ = min(max(value_, minValue_), maxValue_);
}


void SliderDecoratori::serialize(Archive& ar)
{
  ar.serialize(value_, "value", 0);
  ar.serialize(minValue_, "min", 0);
  ar.serialize(maxValue_, "max", 0);
  ar.serialize(step_, "step", 0);
}

}

bool serialize(Archive& ar, ww::SliderDecoratorf& wrapper, const char* name, const char* label)
{
	if(ar.isInPlace())
		return ar(*wrapper.valuePointer_, name, label);

	bool result;
	if(ar.isEdit()){
		result = ar(Serializer(wrapper), name, label);
      if(ar.isOutput())
        wrapper.clip();
	}
	else
		result = ar(wrapper.value_, name, label);

	return result;
}

bool serialize(Archive& ar, ww::SliderDecoratori& wrapper, const char* name, const char* label)
{
	if(ar.isInPlace())
		return ar(*wrapper.valuePointer_, name, label);

	bool result;
	if(ar.isEdit()){
      result = ar(Serializer(wrapper), name, label);
      if(ar.isOutput())
        wrapper.clip();
	}
	else
		result = ar(wrapper.value_, name, label);

	return result;
}

