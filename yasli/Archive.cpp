/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "Archive.h"
#include "Serializer.h"
#include "Config.h"
#include "TypeID.h"

namespace yasli {

struct MapToArrayBase : ContainerInterface {
	MapInterface& map;

	MapToArrayBase(MapInterface& map) : map(map) {}

	size_t size() const override { return map.size(); }
	size_t resize(size_t size) override { 
		// DO NOTHING
		return this->size();
	}
	bool isFixedSize() const override { return false; }

	void* pointer() const override { return map.pointer(); }
	TypeID elementType() const override { return {}; }
	TypeID containerType() const override { return map.containerType(); }
	bool next() override { return map.next(); }
	void* elementPointer() const override { return nullptr; }

	operator bool() const override { return !map.isEmpty(); }
	void serializeNewElement(Archive& ar, const char* name = "", const char* label = 0) const override {
		struct KeyValueNew {
			const MapToArrayBase& map;

			KeyValueNew(const MapToArrayBase& map) : map(map) {}
			void serialize(Archive& ar) {
				map.map.serializeDefaultKey(ar, "key", "^");
				map.map.serializeDefaultValue(ar, "value", "^");
			}
		};
		KeyValueNew keyValue(*this);
		ar(keyValue, name, label);
	};
};

struct MapToArraySerializer : MapToArrayBase {
	bool operator()(Archive& ar, const char* name, const char* label) override {
		struct KeyValueSerializer {
			MapToArrayBase& map;

			KeyValueSerializer(MapToArrayBase& map) : map(map) {}

			void serialize(Archive& ar) {
				map.map.serializeKey(ar, "key", "^");
				map.map.serializeValue(ar, "value", "^");
			}
		};

		KeyValueSerializer keyValue(*this);
		return ar(keyValue, name, label);
	};
	MapToArraySerializer(MapInterface& map) : MapToArrayBase(map) {}
};

struct MapToArrayDeserializer : MapToArrayBase {
	bool operator()(Archive& ar, const char* name, const char* label) override {
		struct KeyValueDeserializer {
			MapToArrayBase& map;

			KeyValueDeserializer(MapToArrayBase& map) : map(map) {}

			void serialize(Archive& ar) {
				map.map.deserializeNewKey(ar, "key", "^");
				map.map.deserializeNewValue(ar, "value", "^");
			}
		};
		KeyValueDeserializer keyValue(*this);
		return ar(keyValue, name, label);
	};
	MapToArrayDeserializer(MapInterface& map) : MapToArrayBase(map) {}
};

bool Archive::operator()(MapInterface& ser, const char* name, const char* label) {
	// for archives that do not implement MapInterface support 
	// we translate MapInterface into a ContainerInterface
	if (isOutput()) {
		MapToArraySerializer converter(ser);
		return operator()(static_cast<ContainerInterface&>(converter), name, label);
	}
	if (isInput()) {
		MapToArrayDeserializer converter(ser);
		return operator()(static_cast<ContainerInterface&>(converter), name, label);
	}
	return false;
}

}
