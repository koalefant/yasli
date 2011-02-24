#pragma once

#include "Archive.h"
#include "MemoryWriter.h"

namespace yasli {

class InPlaceOArchive : public Archive
{
public:
	InPlaceOArchive(bool resolveGraphs);
	bool save(const char* filename);

	bool operator()(bool& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(std::string& value, const char* name, const char* label);
	bool operator()(std::wstring& value, const char* name, const char* label);
	bool operator()(float& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(double& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(int& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(unsigned int& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(short& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(unsigned short& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(long long& value, const char* name, const char* label) { return findOffset(0, &value, name); }

	bool operator()(signed char& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(unsigned char& value, const char* name, const char* label) { return findOffset(0, &value, name); }
	bool operator()(char& value, const char* name, const char* label) { return findOffset(0, &value, name); }

	bool operator()(const Serializer &ser, const char* name, const char* label);
	bool operator()(ContainerSerializationInterface &container, const char* name, const char* label);
	bool operator()(const PointerSerializationInterface& ptr, const char* name, const char* label);
	void inPlacePointer(void** pointer, size_t offset);

	using Archive::operator();
private:
	struct Chunk
	{
		void* address;
		size_t position;
		size_t size;
		size_t count; // used for continguous arrays
		TypeID typeID;
	};

	template<class T>
	bool findOffset(size_t* offset, const T* member, const char* name) {
		return findOffset(offset, (void*)member, sizeof(T), name);
	}
	bool findOffset(size_t* offset, const void* addr, size_t size, const char* name);
	void pushPointer(size_t offset, size_t pointToOffset);
	void appendChunk(void* address, TypeID type, size_t sizeOf, size_t count);
	void pushChunk(size_t position, void* address, TypeID type, size_t sizeOf, size_t count);
	void popChunk();
	void rewritePointers();

	MemoryWriter buffer_; // buffer used to store output memory image

	typedef std::map<void*, size_t> PointerToOffset;
	PointerToOffset pointerToOffset_; // mapping from original addresses of all structures to ther locations inside image
	typedef std::vector<size_t> PointerOffsets;
	PointerOffsets pointerOffsets_; 

	typedef std::vector<Chunk> Stack; 
	Stack stack_; // current stack of chunks, used to verify members addresses

	bool resolveGraphs_;
};

}
