#include "yasli/decorators/FileOpen.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

namespace yasli {

YASLI_INLINE bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, FileOpen& value, const char* name, const char* label)
{
	if (value.pathPointer == nullptr)
		return false;
	if (ar.isEdit())
		return ar(Serializer::forEdit(value), name, label);
	else
		return ar(*value.pathPointer, name, label);
}

}

