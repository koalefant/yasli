#include "stdafx.h"
#include "BinArchive.h"
#include <map>
#include "yasli/MemoryWriter.h"
#include "yasli/TypesFactory.h"
#include "Unicode.h"

using namespace std;

namespace yasli{

static const unsigned int BIN_MAGIC = 0xb1a4c17f;

#ifdef _DEBUG
typedef std::map<unsigned, string> HashMap;
static HashMap hashMap;
#endif

BinOArchive::BinOArchive()
: Archive(false)
{
    clear();
}

void BinOArchive::clear()
{
    stream_.clear();
    stream_.write((const char*)&BIN_MAGIC, sizeof(BIN_MAGIC));
}

size_t BinOArchive::length() const
{ 
    return stream_.position();
}

const char* BinOArchive::buffer()
{
    return (const char*)stream_.buffer();
}

bool BinOArchive::save(const char* filename)
{
    FILE* f = ::yasli::fopen(filename, "wb");
    if(!f)
        return false;

    if(fwrite(buffer(), 1, length(), f) != length())
    {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

inline void BinOArchive::openNode(const char* name)
{
	if(!strlen(name))
		return;

	unsigned hash = calcHash(name, strlen(name));
	stream_.write(hash);

	blockSizeOffsets_.push_back(stream_.position());
	unsigned int size = 0;
	stream_.write(size); // length, shall be overwritten in closeNode

#ifdef _DEBUG
	HashMap::iterator i = hashMap.find(hash);
	if(i != hashMap.end() && i->second != name)
		ASSERT_STR(0, name);
	hashMap[hash] = name;
#endif
}

inline void BinOArchive::closeNode(const char* name)
{
	if(!strlen(name))
		return;

    unsigned offset = blockSizeOffsets_.back();
    blockSizeOffsets_.pop_back();

    unsigned int* blockSize = (unsigned int*)((unsigned char*)stream_.buffer() + offset);
    *blockSize = stream_.position() - offset - sizeof(unsigned);
}

bool BinOArchive::operator()(bool& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(string& value, const char* name, const char* label)
{
    openNode(name);
	stream_ << value.c_str();
	stream_.write(char(0));
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(wstring& value, const char* name, const char* label)
{
	openNode(name);
	stream_ << value.c_str();
	stream_.write(short(0));
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(float& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(double& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(short& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(signed char& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(unsigned char& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(char& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(unsigned short& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(int& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(unsigned int& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(__int64& value, const char* name, const char* label)
{
    openNode(name);
    stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
    openNode(name);
    ser(*this);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
	openNode(name);
	stream_.write(ser.size());

    if(ser.size() > 0)
        do 
        {
            ser(*this, "", "");
        } while (ser.next());

    closeNode(name);
    return true;
}

bool BinOArchive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
    ASSERT_STR(ptr.baseType().registered() && "Writing type with unregistered base", ptr.baseType().name());
    ASSERT_STR(!ptr.get() || ptr.type().registered() && "Writing unregistered type", ptr.type().name());

	openNode(name);

	if(ptr.get()){
		stream_ << ptr.type().name();
		stream_.write(char(0));
		ptr.serializer()(*this);
	}
	else
		stream_.write(char(0));

    closeNode(name);
    return true;
}


//////////////////////////////////////////////////////////////////////////

BinIArchive::BinIArchive()
: Archive(true)
, loadedData_(0)
{
}

BinIArchive::~BinIArchive()
{
	close();
}

bool BinIArchive::load(const char* filename)
{
	close();

	FILE* f = ::yasli::fopen(filename, "rb");
	if(!f)
		return false;
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(length == 0){
		fclose(f);
		return false;
	}
	loadedData_ = new char[length];
	if(fread((void*)loadedData_, 1, length, f) != length || !open(loadedData_, length)){
		close();
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool BinIArchive::open(const char* buffer, size_t size)
{
	if(*(unsigned*)(buffer) != BIN_MAGIC)
		return false;
	buffer += sizeof(unsigned);
	size -= sizeof(unsigned);

	blocks_.push_back(Block(buffer, size));
	return true;
}

void BinIArchive::close()
{
	if(loadedData_)
		delete[] loadedData_;
	loadedData_ = 0;
}

bool BinIArchive::openNode(const char* name)
{
	if(!strlen(name))
		return true;

	Block block(0, 0);
	if(currentBlock().get(name, block)){
		blocks_.push_back(block);
		return true;
	}
	return false;
}

void BinIArchive::closeNode(const char* name) 
{
	if(!strlen(name))
		return;

	ASSERT(currentBlock().validToClose());
	blocks_.pop_back();
}

bool BinIArchive::operator()(bool& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(string& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(wstring& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(float& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(double& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(short& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(unsigned short& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}


bool BinIArchive::operator()(int& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(unsigned int& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(__int64& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(signed char& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(unsigned char& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(char& value, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	ser(*this);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	size_t size;
	read(size);
	ser.resize(size);

	if(ser.size() > 0){
		do
			ser(*this, "", "");
		while(ser.next());
	}
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
	if(!openNode(name))
		return false;

	string typeName;
	read(typeName);
	TypeID type;
	if(!typeName.empty())
		type = TypeID(typeName.c_str());
	if(ptr.type() && (!type || (type != ptr.type())))
		ptr.create(TypeID()); // 0

	if(type && !ptr.get())
		ptr.create(type);

	if(Serializer ser = ptr.serializer())
		ser(*this);

	closeNode(name);
	return true;
}

bool BinIArchive::Block::get(const char* name, Block& block) 
{
	complex_ = true;
	unsigned hashName = calcHash(name, strlen(name));
	const char* currInitial = curr_;
	for(;;){
		unsigned size, hash;
		read(hash);
		read(size);
		
		const char* currPrev = curr_;
		if((curr_ += size) == end_)
			curr_ = begin_;

		ASSERT(curr_ < end_);
		
		if(hash == hashName){
			block = Block(currPrev, size);
			return true;
		}
		
		if(curr_ == currInitial)
			return false;
	}
}

}
