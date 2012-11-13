#pragma once

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

	if(ar.isOutput()){
		if(oldTypeID){
			if(ar(oldTypeID, "")){
				ar(serializer(), "");
			}
			else
				ar.warning("Unable to write typeID!");
		}
	}
	else{
		TypeID typeID;
		if(!ar(typeID, "")){
			if(oldTypeID){
				create(TypeID()); // 0
			}
			return;
		}

		if(oldTypeID && (!typeID || (typeID != oldTypeID)))
			create(TypeID()); // 0

		if(typeID){
			if(!get())
				create(typeID);
			serializer()(ar);
		}
	}	
}

}
// vim:sw=4 ts=4:
