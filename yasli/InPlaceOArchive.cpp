#include "StdAfx.h"
#include "InPlaceOArchive.h"
#include "Files.h" // for yasli::fopen

namespace yasli {

InPlaceOArchive::InPlaceOArchive()
: Archive(INPUT | INPLACE)
{
}

bool InPlaceOArchive::findOffset(size_t* offset, const void* addr, size_t size, const char* name)
{
  ESCAPE(!stack_.empty(), return false);

  if (!stack_.empty())
  {
		Chunk& chunk = *stack_.back();
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

bool InPlaceOArchive::operator()(std::string& value, const char* name, const char* label)
{
	size_t offset;
	if (!findOffset(&offset, value, name))
		return false;
#ifdef _MSC_VER
	bool usesInternalBuffer = value.c_str() >= (char*)&value && value.c_str() < (char*)&value + sizeof(value);
	if (!usesInternalBuffer)
			pointerOffsets_.push_back(offset + 8);
#else
# error Unsupported platform
#endif

	Chunk chunk;
	chunk.address = (void*)value.c_str();
	chunk.position = buffer_.position();
	chunk.size = sizeof(char);
	chunk.typeID = TypeID::get<char>();
	chunk.count = value.size() + 1;

	chunks_.insert(AddressToChunkMap::value_type(chunk.address, chunk));
	buffer_.write(value.c_str(), value.size() + 1);
	return true;
}

bool InPlaceOArchive::operator()(std::wstring& value, const char* name, const char* label)
{
	 return findOffset(0, &value, name);
}

bool InPlaceOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
	Chunk* parent = 0;
	bool isRoot = buffer_.position() == 0;
	if (!stack_.empty())
	{
		parent = stack_.back();
	}

	Chunk chunk;
	chunk.typeID = ser.type();
	if (parent)
	{
		if(!findOffset(&chunk.position, ser.pointer(), ser.size(), name))
			return false;
	}
	else
	{
		// root chunk
		chunk.position = buffer_.position();
	}

	chunk.count = 1;
	chunk.address = ser.pointer();
	chunk.size = ser.size();

	std::pair<AddressToChunkMap::iterator, bool> it = chunks_.insert(AddressToChunkMap::value_type(chunk.address, chunk));

	chunkEnds_[(char*)chunk.address + chunk.size * chunk.count] = &it.first->second;
	stack_.push_back(&it.first->second);
	if (!parent)
		buffer_.write(ser.pointer(), ser.size());
	ser(*this);
	stack_.pop_back();

	if (isRoot)
	{
		// dehydrate pointers
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
			
			AddressToChunkMap::iterator it = chunks_.find(address);
			size_t dehydratedOffset;
			if(it == chunks_.end())
			{
				EndAddressToChunkMap::iterator eit = chunkEnds_.find(address);
				ESCAPE(eit != chunkEnds_.end(), continue);
				dehydratedOffset = eit->second->position + eit->second->size * eit->second->count;
			}
			else
				dehydratedOffset = it->second.position;

			(size_t&)(buffer_.buffer()[offset]) = dehydratedOffset;
		}
	}

	return true;
}

bool InPlaceOArchive::operator()(ContainerSerializationInterface &container, const char* name, const char* label)
{
	container.extractInPlacePointers(*this);

	Stack stack;
	stack.swap(stack_);

	Chunk chunk;
	chunk.typeID = container.type();
	chunk.position = buffer_.position();
	chunk.size = container.elementSize();
	chunk.count = container.size();
	chunk.address = container.elementPointer();

	std::pair<AddressToChunkMap::iterator, bool> it = chunks_.insert(AddressToChunkMap::value_type(chunk.address, chunk));

	chunkEnds_[(char*)chunk.address + chunk.size * chunk.count] = &it.first->second;
	stack_.push_back(&it.first->second);

	if(container.size() > 0){
		buffer_.write(container.elementPointer(), chunk.size * chunk.count);
		do{
			container(*this, "", "");
		}while(container.next());
	}

	stack_.pop_back();

	stack.swap(stack_);
	return true;
}

void InPlaceOArchive::inPlacePointer(void** pointer)
{
	size_t offset;
	ESCAPE(findOffset(&offset, (void*)pointer, sizeof(pointer), "pointer"), return);

	pointerOffsets_.push_back(offset);
}

bool InPlaceOArchive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
	ptr.extractInPlacePointers(*this);

	Stack stack;
	stack.swap(stack_);

	if(ptr.get())
		operator()(ptr.serializer());
	
	stack.swap(stack_);
	return true;
}

}
