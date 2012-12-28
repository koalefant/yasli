#pragma once

#include "yasli/Config.h"
#include "ClassFactoryBase.h"

// Archive.h is supposed to be pre-included

namespace yasli {

inline bool Serializer::operator()(Archive& ar) const{
	YASLI_ESCAPE(serializeFunc_ && object_, return false);
	return serializeFunc_(object_, ar);
}



inline void PointerInterface::serialize(Archive& ar) const
{
	TypeID baseTypeID = baseType();
	TypeID oldTypeID = type();
	ClassFactoryBase* factory = this->factory();

	if(ar.isOutput()){
		if(oldTypeID){
			TypeIDWithFactory pair(oldTypeID, factory);
			if(ar(pair, "")){
				ar(serializer(), "");
			}
			else
				ar.warning("Unable to write typeID!");
		}
	}
	else{
		TypeIDWithFactory pair(TypeID(), factory);
		if(!ar(pair, "")){
			if(oldTypeID){
				create(TypeID()); // 0
			}
			return;
		}

		if(oldTypeID && (!pair.type || (pair.type != oldTypeID)))
			create(TypeID()); // 0

		if(pair.type){
			if(!get())
				create(pair.type);
#if YASLI_NO_EXTRA_BLOCK_FOR_POINTERS
			serializer()(ar);
#else
			ar(serializer(), "");
#endif
		}
	}	
}

}
// vim:sw=4 ts=4:
