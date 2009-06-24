#include "stdafx.h"
#include "RangedWrapper.h"

#include "yasli/Archive.h"

void RangedWrapperf::clip()
{
	value_ = range_.clip(value_);
}

void RangedWrapperf::serialize(Archive& ar)
{
  ar.serialize(value_, "value", 0);
  ar.serialize(range_, "range", 0);
  ar.serialize(step_, "step", 0);
}

bool serialize(Archive& ar, RangedWrapperf& wrapper, const char* name, const char* label)
{
	if(ar.inPlace())
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

void RangedWrapperi::clip()
{
	value_ = range_.clip(value_);
}


void RangedWrapperi::serialize(Archive& ar)
{
  ar.serialize(value_, "value", 0);
  ar.serialize(range_, "range", 0);
  ar.serialize(step_, "step", 0);
}
bool serialize(Archive& ar, RangedWrapperi& wrapper, const char* name, const char* label)
{
	if(ar.inPlace())
		return ar(*wrapper.valuePointer_, name, label);

	bool result;
	if(ar.isEdit()){
      result = ar( Serializer(wrapper), name, label);
      if(ar.isOutput())
        wrapper.clip();
	}
	else
		result = ar(wrapper.value_, name, label);

	return result;
}
