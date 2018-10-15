/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <limits>
#include <math.h>
#include <float.h>
#include "yasli/STL.h"
#include "yasli/BlackBox.h"
#include "yasli/Config.h"
#include "GasonIArchive.h"
#include "gason.h"

namespace yasli{

const char* tagNames[] = {
	"JSON_NUMBER",
	"JSON_STRING",
	"JSON_ARRAY",
	"JSON_OBJECT",
	"JSON_TRUE",
	"JSON_FALSE",
	"JSON_NULL",
};

// ---------------------------------------------------------------------------

GasonIArchive::GasonIArchive()
: Archive(INPUT | TEXT | VALIDATION)
{
}

GasonIArchive::~GasonIArchive()
{
}

bool GasonIArchive::openDestructive(char* buffer, size_t length)
{
	if(!length)
		return false;

	char* endPtr = buffer + length;
	int result = jsonParse(buffer, &endPtr, &stackBottom_.value, allocator_);
	stackBottom_.iterator = begin(stackBottom_.value);
	if (!(stackBottom_.iterator != end(stackBottom_.value))) {
		printf("empty json root!\n");
	}

	stack_ = nullptr;//&stackBottom_;
	if (result != JSON_OK) {
		fprintf(stderr, "json error: %s\n", jsonStrError(result));
	}
	return result == JSON_OK;
}

bool GasonIArchive::findValue(JsonValue& v, const char* name)
{
	if(stack_ == nullptr) {
		v = stackBottom_.value;
		stack_ = &stackBottom_;
		//printf("findValue %s->root\n", name);
		return true;
	}
	Level& level = *stack_;
	if (level.isKeyValue) {
		//printf("findValue %s->isKeyValue\n", name);
		return true;
	}
	JsonIterator it = level.iterator;
	JsonIterator beginIterator = it;
	JsonIterator e = end(level.value);
	if (!(it != e)) {
		//printf("findValue %s->EMPTY\n", name);
		return false;
	}
	if (level.value.getTag() == JSON_OBJECT) {
		if (name[0] != '\0') {
			if (it != e) {
				do {
					bool match = strcmp(it->key, name) == 0;
					//printf(" tried %s\n", it->key);
					JsonIterator current = it;
					++it;
					if (!(it != e))
						it = begin(level.value);
					if (match) {
						level.iterator = it;
						v = current->value;
						//printf("findValue %s->found %s\n", name, tagNames[v.getTag()]);
						return true;
					}
				} while(it != beginIterator);
			}
			//printf("findValue %s->NOT found\n", name);
			return false;
		} else {
			if (level.iterator != end(level.value)) {
				v = level.iterator->value;
				++level.iterator;
				//printf("findValue ->found %s\n", tagNames[v.getTag()]);
				return true;
			} else {
				//printf("findValue ->NOT found\n");
				return false;
			}
		}
	}
	if (level.value.getTag() == JSON_ARRAY) {
		if (!(it != e)) {
			return false;
		}
		v = it->value;
		++it;
		level.iterator = it;
		return true;
	}
	return false;
}

bool GasonIArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_OBJECT){
		Level level(stack_);
		level.value = v;
		level.iterator = begin(v);
		level.isContainer = v.getTag() == JSON_ARRAY;
		level.isDictionary = v.getTag() == JSON_OBJECT;

        if (ser)
            ser(*this);

        return true;
    } else {
		if (v.getTag() >= 0 && v.getTag() <= JSON_NULL ) {
			printf("Expecting JSON_OBJECT %s, got %s", name, tagNames[v.getTag()]);
		} else {
			printf("Expecting JSON_OBJECT %s, got %d", name, v.getTag());
		}
	}
    return false;
}

bool GasonIArchive::operator()(const BlackBox& box, const char* name, const char* label)
{
	return false;
}

bool GasonIArchive::operator()(PointerInterface& ser, const char* name, const char* label)
{
	return false;
	/*
	JsonValue v;
	if (findValue(v, name)) {
		Level level(stack_);
		level.start = token_.end;
		level.isKeyValue = true;

		readToken();
		if (isName(token_)) {
			if(checkStringValueToken()){
				string typeName;
				unescapeString(unescapeBuffer_, typeName, token_.start + 1, token_.end - 1);

				if (typeName != ser.registeredTypeName())
					ser.create(typeName.c_str());
				readToken();
				expect(':');
				operator()(ser.serializer(), "", 0);
			}
		}
		else {
			putToken();

			ser.create("");				
		}
		closeBracket();
		return true;
	}
	return false;
	*/
}


bool GasonIArchive::operator()(ContainerInterface& ser, const char* name, const char* label)
{
	JsonValue v;
	if (findValue(v, name) && (v.getTag() == JSON_ARRAY || v.getTag() == JSON_OBJECT)){
		Level level(stack_);
		level.isContainer = v.getTag() == JSON_ARRAY;
		level.isDictionary = v.getTag() == JSON_OBJECT;
		level.value = v;
		level.iterator = begin(v);

		std::size_t size = ser.size();
		std::size_t index = 0;
		std::size_t jsonLength = 0;
		for (auto i: v) {
			++jsonLength;
		}
		ser.resize(jsonLength);
		//printf("array length: %d\n", int(jsonLength));

		for (; index < jsonLength; ++index) {
			//printf(" element %d\n", int(index));
			if (!ser(*this, "", "")) {
				//printf(" element %d FAILED\n", int(index));
				break;
			}
			if (index != jsonLength - 1) {
				ser.next();
			}
		}
		if(size > index)
			ser.resize(index);
		return true;
	}
    return false;
}

bool GasonIArchive::operator()(MapInterface& ser, const char* name, const char* label)
{
	return false;
	/*
    if (findName(name)) {
		Token bracketToken = token_;

		if (ser.keySerializesToString()) {
			// expect { KEY: VALUE, ... }
			if (!openBracket()) {
				error(nullptr, TypeID(), "Expecting opening brace '{'");
				return false;
			}

			Level level(stack_);
			level.isKeyValue = true;
			level.start = token_.end;

			bool missingComma = false;
			bool first = true;
			while(true){
				readToken();
				if (!first) {
					if (token_ == ',') {
						readToken();
					} else {
						missingComma = true;
					}
				}
				if (token_ == '}')
					break;
				putToken();
				ser.deserializeNewKey(*this, "", "");
				readToken();
				if (!expect(':'))
					break;
				ser.deserializeNewValue(*this, "", "");
				ser.next();
				first = false;
			}

		} else {
			// expect [ { "key": KEY, "value": VALUE }, ... ]
			if (!openContainerBracket()) {
				warning(nullptr, TypeID(), "Expecting an opening square bracket");
			}
			Level outer_level(stack_);
			outer_level.isContainer = true;
			outer_level.start = token_.end;

			Level level(stack_);
			level.isContainer = false;
			level.isKeyValue = true;
			level.start = token_.end;

			bool first = true;
			bool missingComma = false;
			while(true){
				readToken();
				if (!first) {
					if (token_ == ',') {
						readToken();
					} else {
						missingComma = true;
					}
				}
				if (token_ == '{') {
					if (missingComma) {
						error(nullptr, TypeID(), "Missing comma before block");
					}
					if (!ser.deserializeNewKey(*this, "key", "")) {
						error(nullptr, TypeID(), "Missing expected key");
						break;
					}
					readToken();
					if (token_ != ',') {
						error(nullptr, TypeID(), "Expected comma");
						putToken();
					}
					if (!ser.deserializeNewValue(*this, "value", "") ){
						error(nullptr, TypeID(), "Missing expected value");
						break;
					}
					ser.next();
					readToken();
					if (!expect('}'))
						break;
				} else if (token_ == ']') {
					break;
				} else if(!token_) {
					error(nullptr, TypeID(), "Reached end of file while reading container");
					return false;
				} else {
					error(nullptr, TypeID(), "Unexpected token, expecting '{'");
				}
				first = false;
			}
			return true;
		}
	}
    return false;
	*/
}

bool GasonIArchive::operator()(i32& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name)){
		if (v.getTag() == JSON_NUMBER) {
			value = i32(v.toNumber());
			return true;
		} else {
			return false;
		}
    }
    return false;

}

bool GasonIArchive::operator()(u32& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name)){
		if (v.getTag() == JSON_NUMBER) {
			value = u32(v.toNumber());
			return true;
		} else {
			return false;
		}
    }
    return false;
}


bool GasonIArchive::operator()(i16& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name)){
		if (v.getTag() == JSON_NUMBER) {
			value = i16(v.toNumber());
			return true;
		} else {
			return false;
		}
    }
    return false;
}

bool GasonIArchive::operator()(u16& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name)){
		if (v.getTag() == JSON_NUMBER) {
			value = u16(v.toNumber());
			return true;
		} else {
			return false;
		}
    }
    return false;
}

bool GasonIArchive::operator()(i64& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_NUMBER){
		value = i64(v.toNumber());
		return true;
    }
    return false;
}

bool GasonIArchive::operator()(u64& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name)){
		if (v.getTag() == JSON_NUMBER) {
			value = u64(v.toNumber());
			return true;
		} else {
			return false;
		}
    }
    return false;
}

bool GasonIArchive::operator()(float& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_NUMBER){
		value = float(v.toNumber());
    }
    return false;
}

bool GasonIArchive::operator()(double& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_NUMBER){
		value = v.toNumber();
		return true;
    }
    return false;
}

bool GasonIArchive::operator()(StringInterface& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_STRING){
		value.set(v.toString());
		return true;
    }
    return false;
}


bool GasonIArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
	return false;
}

bool GasonIArchive::operator()(bool& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name)){
		auto tag =v.getTag(); 
		if (tag == JSON_TRUE) {
			value = true;
			return true;
		} else if (tag == JSON_FALSE) {
			value = false;
			return true;
		}
    }
    return false;
}

bool GasonIArchive::operator()(i8& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_NUMBER){
		value = i8(v.toNumber());
		return true;
    }
    return false;

}

bool GasonIArchive::operator()(u8& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_NUMBER){
		value = u8(v.toNumber());
		return true;
    }
    return false;
}

bool GasonIArchive::operator()(char& value, const char* name, const char* label)
{
	JsonValue v;
    if (findValue(v, name) && v.getTag() == JSON_NUMBER){
		value = char(v.toNumber());
		return true;
    }
    return false;
}

void GasonIArchive::setDebugFilename(const char* filename) {
	filename_ = filename;
}

void GasonIArchive::validatorMessage(bool error, const void* handle, const TypeID& type, const char* message) {
}

}
// vim:ts=4 sw=4:
