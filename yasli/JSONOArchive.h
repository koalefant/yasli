/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include "yasli/Archive.h"
#include "Pointers.h"

namespace yasli{

class MemoryWriter;

class JSONOArchive : public Archive{
public:
	// header = 0 - default header, use "" to omit
	JSONOArchive(int textWidth = 80, const char* header = 0);
	~JSONOArchive();

	bool save(const char* fileName);

	const char* c_str() const;    
	size_t length() const;    

	// from Archive:
	bool operator()(bool& value, const char* name = "", const char* label = 0);
	bool operator()(StringInterface& value, const char* name = "", const char* label = 0);
	bool operator()(WStringInterface& value, const char* name = "", const char* label = 0);
	bool operator()(float& value, const char* name = "", const char* label = 0);
	bool operator()(double& value, const char* name = "", const char* label = 0);
	bool operator()(short& value, const char* name = "", const char* label = 0);
	bool operator()(unsigned short& value, const char* name = "", const char* label = 0);
	bool operator()(int& value, const char* name = "", const char* label = 0);
	bool operator()(unsigned int& value, const char* name = "", const char* label = 0);
	bool operator()(long long& value, const char* name = "", const char* label = 0);
	bool operator()(unsigned long long& value, const char* name = "", const char* label = 0);

	bool operator()(char& value, const char* name = "", const char* label = 0);
	bool operator()(signed char& value, const char* name = "", const char* label = 0);
	bool operator()(unsigned char& value, const char* name = "", const char* label = 0);

	bool operator()(const Serializer& ser, const char* name = "", const char* label = 0);
	bool operator()(ContainerInterface& ser, const char* name = "", const char* label = 0);
	bool operator()(KeyValueInterface& keyValue, const char* name = "", const char* label = 0);
	bool operator()(PointerInterface& ser, const char* name = "", const char* label = 0);
	// ^^^

	using Archive::operator();
private:
	void openBracket();
	void closeBracket();
	void openContainerBracket();
	void closeContainerBracket();
	void placeName(const char* name);
	void placeIndent(bool putComma = true);
	void placeIndentCompact(bool putComma = true);

	bool joinLinesIfPossible();

	struct Level{
		Level(bool _isContainer, std::size_t position, int column)
		: isContainer(_isContainer)
		, isKeyValue(false)
		, isDictionary(false)
		, startPosition(position)
		, indentCount(-column)
		, elementIndex(0)
		, nameIndex(0)
		{}
		bool isKeyValue;
		bool isContainer;
		bool isDictionary;
		std::size_t startPosition;
		int nameIndex;
		int elementIndex;
		int indentCount;
	};

	typedef std::vector<Level> Stack;
	Stack stack_;
	std::auto_ptr<MemoryWriter> buffer_;
	const char* header_;
	int textWidth_;
	std::string fileName_;
	int compactOffset_;
	bool isKeyValue_;
};

}
