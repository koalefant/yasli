/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "Config.h"
#include "yasli/Archive.h"
#include "gason.h"

namespace yasli{

class GasonIArchive : public Archive{
public:
	GasonIArchive();
	~GasonIArchive();

	bool openDestructive(char* buffer, size_t length);
	// filename that will be include with errors produced with error() calls
	void setDebugFilename(const char* filename);
	// allows to disable warnings (enabled by default)
	void setDisableWarnings(bool disableWarnings) { disableWarnings_ = disableWarnings; }
	// enabled by default, can be disabled for performance reasons
	void setWarnAboutUnusedFields(bool warn) { warnAboutUnusedFields_ = warn; }
	int unusedFieldCount() const { return unusedFieldCount_; }

	bool operator()(bool& value, const char* name = "", const char* label = 0) override;
	bool operator()(char& value, const char* name = "", const char* label = 0) override;
	bool operator()(float& value, const char* name = "", const char* label = 0) override;
	bool operator()(double& value, const char* name = "", const char* label = 0) override;
	bool operator()(i8& value, const char* name = "", const char* label = 0) override;
	bool operator()(i16& value, const char* name = "", const char* label = 0) override;
	bool operator()(i32& value, const char* name = "", const char* label = 0) override;
	bool operator()(i64& value, const char* name = "", const char* label = 0) override;
	bool operator()(u8& value, const char* name = "", const char* label = 0) override;
	bool operator()(u16& value, const char* name = "", const char* label = 0) override;
	bool operator()(u32& value, const char* name = "", const char* label = 0) override;
	bool operator()(u64& value, const char* name = "", const char* label = 0) override;

	bool operator()(StringInterface& value, const char* name = "", const char* label = 0) override;
	bool operator()(WStringInterface& value, const char* name = "", const char* label = 0) override;
	bool operator()(const Serializer& ser, const char* name = "", const char* label = 0) override;
	bool operator()(const BlackBox& ser, const char* name = "", const char* label = 0) override;
	bool operator()(ContainerInterface& ser, const char* name = "", const char* label = 0) override;
	bool operator()(MapInterface& ser, const char* name = "", const char* label = 0) override;
	bool operator()(PointerInterface& ser, const char* name = "", const char* label = 0) override;

	void validatorMessage(bool error, const void* handle, const TypeID& type, const char* message) override;

	using Archive::operator();
private:
	bool findValue(JsonValue& v, const char* name);
	bool openBracket();
	bool closeBracket();

	bool openContainerBracket();
	bool closeContainerBracket();

	struct Level{
		Level*& stackPointer;
		Level* parent;

		JsonValue value;
		JsonIterator iterator;
		bool isContainer{ false };
		bool isKeyValue{ false };
		bool isDictionary{ false };

		Level(Level*& stackPointer)
		: stackPointer(stackPointer) {
			parent = stackPointer;
			stackPointer = this;
		}
		~Level() {
			stackPointer = parent;
		}
	};
	Level* stack_{ nullptr };
	Level stackBottom_{ stack_ };
	JsonAllocator allocator_;

	string filename_;

	bool disableWarnings_{ false };
	bool warnAboutUnusedFields_{ false };
	int unusedFieldCount_{ 0 };
	bool free_{ false };
};

}
