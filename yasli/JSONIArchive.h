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
#include "Token.h"
#include <memory>

namespace yasli{

class MemoryReader;

class JSONIArchive : public Archive{
public:
	JSONIArchive();
	~JSONIArchive();

	bool load(const char* filename);
	bool open(const char* buffer, size_t length);
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
	bool findName(const char* name, Token* outName = 0, bool untilEndOfBlockOnly = false);
	bool openBracket();
	bool closeBracket();

	bool openContainerBracket();
	bool closeContainerBracket();

	void checkValueToken();
	void checkIntegerToken();
	bool checkStringValueToken();
	void readToken();
	void putToken();
	int line(int * column_no, const char* position) const; 
	bool isName(Token token) const;

	bool expect(char token);
	void skipBlock();

	struct Level{
		Level*& stackPointer;
		Level* parent;

		const char* start;
		const char* firstToken;
		int fieldIndex{ 0 };
		bool isContainer{ false };
		bool isKeyValue{ false };
		bool isDictionary{ false };
		bool parsedBlock{ false };
		std::vector<Token> names;

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

	std::unique_ptr<MemoryReader> reader_;
	Token token_;
	std::vector<char> unescapeBuffer_;
	string filename_;

	bool disableWarnings_{ false };
	bool warnAboutUnusedFields_{ false };
	int unusedFieldCount_{ 0 };
	const char* buffer_{ nullptr };
	const char* bufferEnd_{ nullptr };
	bool free_{ false };
};

double parseFloat(const char* s);

}
