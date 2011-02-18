#include "StdAfx.h"
#include "InPlaceOArchive.h"
#include "TypesFactory.h"

namespace yasli {

InPlaceOArchive::InPlaceOArchive(bool resolveGraphs)
: Archive(INPUT | INPLACE)
, resolveGraphs_(resolveGraphs)
{
}

bool InPlaceOArchive::findOffset(size_t* offset, const void* addr, size_t size, const char* name)
{
	ESCAPE(!stack_.empty(), return false);

	if (!stack_.empty())
	{
		Chunk& chunk = stack_.back();
		if (addr >= chunk.address &&
			((const char*)addr + size) <= (const char*)chunk.address + chunk.size * chunk.count)
		{
			if (offset)
				*offset = chunk.position + ((const char*)addr - (const char*)chunk.address);
			return true;
		}
		else
		{
			MemoryWriter msg;
			msg << "Address of member '" << name << "' lies out of '" << chunk.typeID.name() << "' struct. Are you serializing a stack variable?";
			warning(msg.c_str());
			ASSERT_STR(0, msg.c_str());
			return false;
		}
	}
	return false;
}

bool InPlaceOArchive::save(const char* filename)
{
	MemoryWriter buf;
	buf.write("YINPA32", 8);

	int headerSize = pointerOffsets_.size() * sizeof(size_t);
	buf.write(headerSize);
	if (!pointerOffsets_.empty())
		buf.write((const char*)&pointerOffsets_[0], headerSize);

	buf.write(buffer_.position());
	buf.write(buffer_.buffer(), buffer_.position());

	FILE* f = fopen(filename, "wb");
	if (!f)
		return false;
	if(fwrite(buf.buffer(), buf.position(), 1, f) != 1)
		return false;
	fclose(f);
	return true;
}

void InPlaceOArchive::pushPointer(size_t offset, size_t pointToOffset)
{
	pointerOffsets_.push_back(offset);
	if (!resolveGraphs_)
	{
		ASSERT(offset + sizeof(void*) <= buffer_.position());
		size_t* ptr = (size_t*)&buffer_.buffer()[offset];
		*ptr = pointToOffset;
	}
}

void InPlaceOArchive::appendChunk(void* address, TypeID type, size_t sizeOf, size_t count)
{
	if (resolveGraphs_)
	{
		pointerToOffset_.insert(PointerToOffset::value_type(address, buffer_.position()));
		pointerToOffset_.insert(PointerToOffset::value_type((char*)address + sizeOf * count, buffer_.position() + sizeOf * count));
	}

	buffer_.write((char*)address, sizeOf * count);
}

void InPlaceOArchive::pushChunk(size_t position, void* address, TypeID type, size_t sizeOf, size_t count)
{
	if (resolveGraphs_)
	{
		pointerToOffset_.insert(PointerToOffset::value_type(address, position));
		pointerToOffset_.insert(PointerToOffset::value_type((char*)address + sizeOf * count, position + sizeOf * count));
	}

	Chunk chunk;
	chunk.typeID = type;
	chunk.count = count;
	chunk.position = position;
	chunk.address = address;
	chunk.size = sizeOf;

	stack_.push_back(chunk);
}

void InPlaceOArchive::popChunk()
{
	stack_.pop_back();
}


bool InPlaceOArchive::operator()(std::string& value, const char* name, const char* label)
{
	size_t offset;
	if (!findOffset(&offset, &value, name))
		return false;
#ifdef _MSC_VER
	bool usesInternalBuffer = value.c_str() >= (const char*)&value && value.c_str() < (const char*)&value + sizeof(value);
	if (!usesInternalBuffer)
		pushPointer(offset + 8, buffer_.position());
#else
# error Unsupported platform
#endif

	appendChunk((void*)value.c_str(), TypeID::get<char>(), sizeof(char), value.size() + 1);
	return true;
}

bool InPlaceOArchive::operator()(std::wstring& value, const char* name, const char* label)
{
	size_t offset;
	if (!findOffset(&offset, &value, name))
		return false;
#ifdef _MSC_VER
	bool usesInternalBuffer = value.c_str() >= (const wchar_t*)&value && value.c_str() < (wchar_t*)&value + sizeof(value) / sizeof(wchar_t);
	if (!usesInternalBuffer)
		pushPointer(offset + 8, buffer_.position());
#else
# error Unsupported platform
#endif

	appendChunk((void*)value.c_str(), TypeID::get<wchar_t>(), sizeof(wchar_t), value.size() + 1);
	return true;
}

void InPlaceOArchive::rewritePointers()
{
	for (size_t i = 0; i < pointerOffsets_.size(); ++i)
	{
		size_t offset = pointerOffsets_[i];
		void* address = (void*&)(buffer_.buffer()[offset]);
		if (address == 0)
		{
			pointerOffsets_.erase(pointerOffsets_.begin() + i);
			--i;
			continue; // skip NULL-pointers
		}

		PointerToOffset::iterator it = pointerToOffset_.find(address);
		ESCAPE(it != pointerToOffset_.end(), continue);
		size_t dehydratedOffset = it->second;

		(size_t&)(buffer_.buffer()[offset]) = dehydratedOffset;
	}
}

bool InPlaceOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
	Chunk* parent = 0;
	bool isRoot = buffer_.position() == 0;
	bool hasParent = !stack_.empty();

	void* address = ser.pointer();
	size_t size = ser.size();
	size_t position = buffer_.position();
	if (hasParent && !findOffset(&position, address, size, name))
		return false;

	pushChunk(position, address, ser.type(), size, 1);
	if (!hasParent)
		buffer_.write(ser.pointer(), ser.size());

	ser(*this);

	popChunk();

	if (isRoot && resolveGraphs_)
		rewritePointers();

	return true;
}

bool InPlaceOArchive::operator()(ContainerSerializationInterface &container, const char* name, const char* label)
{
	container.extractInPlacePointers(*this);

	Stack stack;
	stack.swap(stack_);

	size_t containerSize = container.size();
	pushChunk(buffer_.position(), containerSize ? container.elementPointer() : 0, container.type(), 
						container.elementSize(), containerSize);

	if (containerSize > 0)
	{
		buffer_.write(container.elementPointer(), container.elementSize() * containerSize);

		do {
			container(*this, "", "");
		} while (container.next());
	}

	// popChunk();

	stack.swap(stack_);
	return true;
}

void InPlaceOArchive::inPlacePointer(void** pointer, size_t offset)
{
	size_t memberOffset;
	ESCAPE(findOffset(&memberOffset, (void*)pointer, sizeof(pointer), "pointer"), return);

	pushPointer(memberOffset, buffer_.position() + offset);
}

bool InPlaceOArchive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
	if(!ptr.get())
		return false;

	TypeID derivedType = ptr.type();
	
	const ClassFactoryBase *factory = ClassFactoryManager::the().find(ptr.baseType());
	if (!factory)
	{
		ASSERT_STR(0 && "Base type is not registered", ptr.baseType().name());;
		return false;
	}

	size_t size = factory->sizeOf(derivedType);
	if (size == 0)
	{
		ASSERT_STR(0 && "No type description for type", ptr.type().name());;
		return false;
	}

	ptr.extractInPlacePointers(*this);

	Stack stack;
	stack.swap(stack_);
	
	pushChunk(buffer_.position(), ptr.get(), derivedType, size, 1);

	buffer_.write(ptr.get(), size);

	ptr.serializer()(*this);

	// popChunk();

	stack.swap(stack_);
	return true;
}

}
