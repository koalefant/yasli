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

YASLI_INLINE Context::Context()
: archive(0)
, object(0)
, previousContext(0)
{
}

YASLI_INLINE Context::~Context() {
	if (archive)
		archive->setLastContext(previousContext);
}

// ---------------------------------------------------------------------------

YASLI_INLINE Archive::Archive(int caps)
: lastContext_(0)
, caps_(caps)
, filter_(YASLI_DEFAULT_FILTER)
{
}

YASLI_INLINE Archive::~Archive() {
}

YASLI_INLINE bool Archive::isInPlace() const { return (caps_ & INPLACE) != 0; }
YASLI_INLINE bool Archive::caps(int caps) const { return (caps_ & caps) == caps; }

YASLI_INLINE void Archive::setFilter(int filter){
	filter_ = filter;
}
YASLI_INLINE int Archive::getFilter() const{ return filter_; }
YASLI_INLINE bool Archive::filter(int flags) const{
	YASLI_ASSERT(flags != 0 && "flags is supposed to be a bit mask");
	YASLI_ASSERT(filter_ && "Filter is not set!");
	return (filter_ & flags) != 0;
}

YASLI_INLINE bool Archive::operator()(bool& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(char& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(u8& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(i8& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(i16& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(u16& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(i32& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(u32& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(i64& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(u64& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(float& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(double& value, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(StringInterface& value, const char* name, const char* label)    { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(WStringInterface& value, const char* name, const char* label)    { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(const Serializer& ser, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(const BlackBox& ser, const char* name, const char* label) { notImplemented(); return false; }
YASLI_INLINE bool Archive::operator()(ContainerInterface& ser, const char* name, const char* label) { return false; }
YASLI_INLINE bool Archive::operator()(PointerInterface& ptr, const char* name, const char* label) {
	Serializer ser(ptr);
	return operator()(ser, name, label);
}
YASLI_INLINE bool Archive::operator()(Object& obj, const char* name, const char* label) { return false; }
YASLI_INLINE bool Archive::operator()(CallbackInterface& callback, const char* name, const char* label) { return false; }

YASLI_INLINE bool Archive::operator()(long double& value, const char* name, const char* label) { notImplemented(); return false; }

YASLI_INLINE bool Archive::openBlock(const char* name, const char* label) { return true; }
YASLI_INLINE void Archive::closeBlock() {}

YASLI_INLINE void Archive::warning(const void* handle, const yasli::TypeID& type, const char* format, ...)
{
#if !YASLI_NO_EDITING
	if ((caps_ & VALIDATION) == 0)
		return;
	va_list args;
	va_start(args, format);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	validatorMessage(false, handle, type, buf);
#endif
}

YASLI_INLINE void Archive::error(const void* handle, const yasli::TypeID& type, const char* format, ...)
{
#if !YASLI_NO_EDITING
	if ((caps_ & VALIDATION) == 0)
		return;
	va_list args;
	va_start(args, format);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	validatorMessage(true, handle, type, buf);
#endif
}

YASLI_INLINE void Archive::doc(const char* docString)
{
#if !YASLI_NO_EDITING
	if (caps_ & DOCUMENTATION)
		documentLastField(docString); 
#endif
}

YASLI_INLINE void* Archive::contextByType(const TypeID& type) const {
	for (Context* current = lastContext_; current != 0; current = current->previousContext)
		if (current->type == type)
			return current->object;
	return 0;
}

YASLI_INLINE Context* Archive::setLastContext(Context* context) {
	Context* previousContext = lastContext_;
	lastContext_ = context;
	return previousContext;
}
YASLI_INLINE Context* Archive::lastContext() const{
	return lastContext_;
}

YASLI_INLINE void Archive::validatorMessage(bool error, const void* handle, const TypeID& type, const char* message) {
}

YASLI_INLINE void Archive::documentLastField(const char* text) {
}

YASLI_INLINE void Archive::notImplemented() {
	YASLI_ASSERT(0 && "Not implemented!");
}

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

YASLI_INLINE bool Archive::operator()(MapInterface& ser, const char* name, const char* label) {
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
