#pragma once

// Для тегов используется 16-бит xor-hash с проверкой уникальности в debug'e
// Размер блока 8, 16, 32 бита автоматически

#include "Archive.h"
#include "MemoryWriter.h" 

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
	const char* buffer();
	bool save(const char* fileName);

	bool operator()(bool& value, const char* name, const char* label);
	bool operator()(std::string& value, const char* name, const char* label);
	bool operator()(std::wstring& value, const char* name, const char* label);
	bool operator()(float& value, const char* name, const char* label);
	bool operator()(double& value, const char* name, const char* label);
	bool operator()(int& value, const char* name, const char* label);
	bool operator()(unsigned int& value, const char* name, const char* label);
	bool operator()(short& value, const char* name, const char* label);
	bool operator()(unsigned short& value, const char* name, const char* label);
	bool operator()(__int64& value, const char* name, const char* label);

	bool operator()(signed char& value, const char* name, const char* label);
	bool operator()(unsigned char& value, const char* name, const char* label);
	bool operator()(char& value, const char* name, const char* label);

	bool operator()(const Serializer &ser, const char* name, const char* label);
	bool operator()(ContainerSerializationInterface &ser, const char* name, const char* label);
	bool operator()(const PointerSerializationInterface &ptr, const char* name, const char* label);

	using Archive::operator();

private:
	void openContainer(const char* name, int size, const char* typeName);
	void openNode(const char* name);
	void closeNode(const char* name);

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
	void close();

	bool operator()(bool& value, const char* name, const char* label);
	bool operator()(std::string& value, const char* name, const char* label);
	bool operator()(std::wstring& value, const char* name, const char* label);
	bool operator()(float& value, const char* name, const char* label);
	bool operator()(double& value, const char* name, const char* label);
	bool operator()(short& value, const char* name, const char* label);
	bool operator()(unsigned short& value, const char* name, const char* label);
	bool operator()(int& value, const char* name, const char* label);
	bool operator()(unsigned int& value, const char* name, const char* label);
	bool operator()(__int64& value, const char* name, const char* label);

	bool operator()(signed char& value, const char* name, const char* label);
	bool operator()(unsigned char& value, const char* name, const char* label);
	bool operator()(char& value, const char* name, const char* label);

	bool operator()(const Serializer& ser, const char* name, const char* label);
	bool operator()(ContainerSerializationInterface& ser, const char* name, const char* label);
	bool operator()(const PointerSerializationInterface& ptr, const char* name, const char* label);

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
			  ASSERT(curr_ + size <= end_);
			  memcpy(data, curr_, size);
			  curr_ += size;	
		  }

		  template<class T>
		  void read(T& x){ read(&x, sizeof(x)); }

		  void read(std::string& s)
		  {
			  ASSERT(curr_ + strlen(curr_) < end_);
			  s = curr_;
			  curr_ += strlen(curr_) + 1;
		  }
		  void read(std::wstring& s)
		  {
			  ASSERT(curr_ + sizeof(wchar_t) * wcslen((wchar_t*)curr_) < end_);
			  s = (wchar_t*)curr_;
			  curr_ += (wcslen((wchar_t*)curr_) + 1) * sizeof(wchar_t);
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

	bool openNode(const char* name);
	void closeNode(const char* name);
	Block& currentBlock() { return blocks_.back(); }
	template<class T>
	void read(T& t) { currentBlock().read(t); }
};


}