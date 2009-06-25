#include "stdafx.h"
#include "BinaryOArchive.h"
#include <xutility>
#include "BinaryNode.h"
#include "yasli/MemoryWriter.h"
#include "yasli/TypesFactory.h"

const unsigned int BINARY_MAGIC = 0xb1a4c17e;

BinaryOArchive::BinaryOArchive(bool writeTypeInfo)
: Archive(false, true)
{
    clear();
}

void BinaryOArchive::clear()
{
    stream_.set(new MemoryWriter);
    stream_->write((const char*)&BINARY_MAGIC, sizeof(BINARY_MAGIC));
}


size_t BinaryOArchive::length() const
{ 
    CHECK(stream_, return 0);
    return stream_->size();
}

void BinaryOArchive::openContainer(const char* name, int size, const char* typeName)
{
    openNode(BINARY_NODE_CONTAINER, name);
	unsigned char typeNameLen = (unsigned char)std::min<int>(255, int(strlen(typeName)));
    stream_->write((const char*)&typeNameLen, 1);
    stream_->write(typeName, typeNameLen);
    stream_->write((const char*)(&size), sizeof(size));
}

void BinaryOArchive::closeContainer()
{
    closeNode();
}

void BinaryOArchive::openStruct(const char *name, const char *typeName)
{
    openNode(BINARY_NODE_STRUCT, name);
	unsigned char typeNameLen = (unsigned char)std::min<int>(255, (int)strlen(typeName));
    stream_->write((const char*)&typeNameLen, 1);
    stream_->write(typeName, typeNameLen);
}

void BinaryOArchive::closeStruct()
{
    closeNode();
}

void BinaryOArchive::write(const char *_string)
{
    size_t length = strlen(_string);
    ASSERT(length < 256 && "Unable to write string longer than 255 chars");
	unsigned char len = (unsigned char)std::min<int>(255, (int)length);
    stream_->write((const char*)&len, 1);
    stream_->write(_string, len);
}

void BinaryOArchive::openNode(BinaryNode _type, const char *name)
{
    blockSizeOffsets_.push_back(stream_->size());
    unsigned int size = 0;
    stream_->write((const char*)&size, sizeof(size)); // length, shall be overwritten in closeNode
    unsigned char type = (unsigned char)(_type);
    stream_->write((const char*)&type, sizeof(type));

    write(name);
}

void BinaryOArchive::closeNode()
{
    unsigned int offset = blockSizeOffsets_.back();
    blockSizeOffsets_.pop_back();

    unsigned int *blockSize = (unsigned int*)((unsigned char*)stream_->buffer() + offset);
    *blockSize = stream_->size() - offset;
}

bool BinaryOArchive::operator()(bool &value, const char *_name)
{
    openNode(BINARY_NODE_BOOL, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(std::string &value, const char *_name)
{
    openNode(BINARY_NODE_STRING, _name);
    stream_->write(value.c_str(), (int)value.length());
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(float &value, const char *_name)
{
    openNode(BINARY_NODE_FLOAT, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(double &value, const char *_name)
{
    openNode(BINARY_NODE_DOUBLE, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(short &value, const char *_name)
{
    openNode(BINARY_NODE_INT16, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(signed char &value, const char *_name)
{
    openNode(BINARY_NODE_SBYTE, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(unsigned char &value, const char *_name)
{
    openNode(BINARY_NODE_BYTE, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(char &value, const char *_name)
{
    openNode(BINARY_NODE_BYTE, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(unsigned short &value, const char *_name)
{
    openNode(BINARY_NODE_UINT16, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}



bool BinaryOArchive::operator()(int &value, const char *_name)
{
    openNode(BINARY_NODE_INT32, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(unsigned int &value, const char *_name)
{
    openNode(BINARY_NODE_UINT32, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(__int64 &value, const char *_name)
{
    openNode(BINARY_NODE_INT64, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}


bool BinaryOArchive::operator()(const Serializer &ser, const char *_name)
{
    openStruct(_name, ser.type().name());

    ser(*this);

    closeStruct();
    return true;
}

bool BinaryOArchive::operator()(ContainerSerializationInterface &ser, const char *_name)
{
    int size = int(ser.size());
    TypeID type = ser.type();
    openContainer(_name, size, type.name());

    //TODO
    //if (!CClassFactoryManager::The().HasDefaultElement(type))
    //    CClassFactoryManager::The().AddDefaultElement(type, ser.CreateDefaultElementSerializer());

    if (size > 0)
        do 
        {
            ser(*this, "", "");
        } while (ser.next());

    closeContainer();
    return true;
}


bool BinaryOArchive::operator()(const PointerSerializationInterface &_ptr, const char *_name)
{
    openNode(BINARY_NODE_POINTER, _name);
    ASSERT_STR(_ptr.baseType().registered() && "Writing type with unregistered base", _ptr.baseType().name());
    //ASSERT_STR(!_ptr.get() || _ptr.type().registered() && "Writing unregistered type", _ptr.type().shortName()); TODO
    ASSERT_STR(!_ptr.get() || _ptr.type().registered() && "Writing unregistered type", _ptr.type().name());

    TypeID baseType = _ptr.baseType();
    write(baseType.name());
    write(_ptr.type().name());
    //write(_ptr.Type().ShortName(baseType)); TODO

    _ptr.serializer()(*this);

    closeNode();
    return true;
}
