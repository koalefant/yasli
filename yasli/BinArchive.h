/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

// Для тегов используется 16-бит xor-hash с проверкой уникальности в debug'e
// Размер блока 8, 16, 32 бита автоматически

#include "yasli/Archive.h"
#include "yasli/MemoryWriter.h" 

namespace yasli{

inline unsigned short calcHash(const char* str)
{
	unsigned short hash = 0;
	const unsigned short* p = (const unsigned short*)(str);
	for(;;){
		unsigned short w = *p++;
		if(!(w & 0xff))
			break;
		hash ^= w;
		if(!(w & 0xff00))
			break;
	}
	return hash;
}

class BinOArchive : public Archive{
public:

	BinOArchive();
	~BinOArchive() {}

	void clear();
	size_t length() const;
	const char* buffer() const { return stream_.buffer(); }
	bool save(const char* fileName);

	bool operator()(bool& value, const char* name, const char* label);
	bool operator()(StringInterface& value, const char* name, const char* label);
	bool operator()(WStringInterface& value, const char* name, const char* label);
	bool operator()(float& value, const char* name, const char* label);
	bool operator()(double& value, const char* name, const char* label);
	bool operator()(int& value, const char* name, const char* label);
	bool operator()(unsigned int& value, const char* name, const char* label);
	bool operator()(short& value, const char* name, const char* label);
	bool operator()(unsigned short& value, const char* name, const char* label);
	bool operator()(long long& value, const char* name, const char* label);
	bool operator()(unsigned long long& value, const char* name, const char* label);

	bool operator()(signed char& value, const char* name, const char* label);
	bool operator()(unsigned char& value, const char* name, const char* label);
	bool operator()(char& value, const char* name, const char* label);

	bool operator()(const Serializer &ser, const char* name, const char* label);
	bool operator()(ContainerInterface &ser, const char* name, const char* label);
	bool operator()(PointerInterface &ptr, const char* name, const char* label);

	using Archive::operator();

private:
	void openContainer(const char* name, int size, const char* typeName);
	void openNode(const char* name, bool size8 = true);
	void closeNode(const char* name, bool size8 = true);

	std::vector<unsigned int> blockSizeOffsets_;
	MemoryWriter stream_;
};

//////////////////////////////////////////////////////////////////////////

class BinIArchive : public Archive{
public:

	BinIArchive();
	~BinIArchive();

	bool load(const char* fileName);
	bool open(const char* buffer, size_t length); // не копирует буффер!!!
	bool open(const BinOArchive& ar) { return open(ar.buffer(), ar.length()); }
	void close();

	bool operator()(bool& value, const char* name, const char* label);
	bool operator()(StringInterface& value, const char* name, const char* label);
	bool operator()(WStringInterface& value, const char* name, const char* label);
	bool operator()(float& value, const char* name, const char* label);
	bool operator()(double& value, const char* name, const char* label);
	bool operator()(short& value, const char* name, const char* label);
	bool operator()(unsigned short& value, const char* name, const char* label);
	bool operator()(int& value, const char* name, const char* label);
	bool operator()(unsigned int& value, const char* name, const char* label);
	bool operator()(long long& value, const char* name, const char* label);
	bool operator()(unsigned long long& value, const char* name, const char* label);

	bool operator()(signed char& value, const char* name, const char* label);
	bool operator()(unsigned char& value, const char* name, const char* label);
	bool operator()(char& value, const char* name, const char* label);

	bool operator()(const Serializer& ser, const char* name, const char* label);
	bool operator()(ContainerInterface& ser, const char* name, const char* label);
	bool operator()(PointerInterface& ptr, const char* name, const char* label);

	using Archive::operator();

private:
	class Block
	{
	public:
		Block(const char* data, int size) : 
		  begin_(data), curr_(data), end_(data + size), complex_(false) {}

		  bool get(const char* name, Block& block);

		  void read(void *data, int size)
		  {
			  if(curr_ + size <= end_){
				memcpy(data, curr_, size);
				curr_ += size;	
			  }
			  else
				  YASLI_ASSERT(0);
		  }

		  template<class T>
		  void read(T& x){ read(&x, sizeof(x)); }

		  void read(std::string& s)
		  {
			  if(curr_ + strlen(curr_) < end_){
				s = curr_;
				curr_ += strlen(curr_) + 1;
			  }
			  else
				  YASLI_ASSERT(0);
		  }
		  void read(std::wstring& s)
		  {
			  if(curr_ + sizeof(wchar_t) * wcslen((wchar_t*)curr_) < end_){
				s = (wchar_t*)curr_;
				curr_ += (wcslen((wchar_t*)curr_) + 1) * sizeof(wchar_t);
			  }
			  else
				  YASLI_ASSERT(0);
		  }

		  unsigned int readPackedSize();

		  bool validToClose() const { return complex_ || curr_ == end_; } // Простые блоки должны быть вычитаны точно
	
	private:
		const char* begin_;
		const char* end_;
		const char* curr_;
		bool complex_;
	};

	typedef std::vector<Block> Blocks;
	Blocks blocks_;
	const char* loadedData_;
	string stringBuffer_;
	wstring wstringBuffer_;

	bool openNode(const char* name);
	void closeNode(const char* name, bool check = true);
	Block& currentBlock() { return blocks_.back(); }
	template<class T>
	void read(T& t) { currentBlock().read(t); }
};

}
