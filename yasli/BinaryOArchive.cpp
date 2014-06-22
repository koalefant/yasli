/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/ClassFactory.h"
#include "BinaryOArchive.h"
//#include <xutility>
#include "BinaryNode.h"
#include "yasli/MemoryWriter.h"
#include <algorithm>

namespace yasli{

static const unsigned int BINARY_MAGIC = 0xb1a4c17e;

BinaryOArchive::BinaryOArchive(bool pretendToBeEdit)
: Archive(OUTPUT | (pretendToBeEdit ? EDIT : 0))
{
    clear();
}

void BinaryOArchive::clear()
{
    stream_.reset(new MemoryWriter);
    stream_->write((const char*)&BINARY_MAGIC, sizeof(BINARY_MAGIC));
}


size_t BinaryOArchive::length() const
{ 
    YASLI_ESCAPE(stream_.get() != 0, return 0);
    return stream_->position();
}

const char* BinaryOArchive::buffer()
{
    YASLI_ESCAPE(stream_.get() != 0, return 0);
    return stream_->buffer();
}

bool BinaryOArchive::save(const char* filename)
{
    FILE* f = fopen(filename, "wb");
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
    YASLI_ASSERT(length < 256 && "Unable to write string longer than 255 chars");
	unsigned char len = (unsigned char)std::min<int>(255, (int)length);
    stream_->write((const char*)&len, 1);
    stream_->write(_string, len);
}

void BinaryOArchive::openNode(BinaryNode _type, const char *name)
{
    blockSizeOffsets_.push_back((unsigned int)stream_->position());
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

    unsigned int *blockSize = (unsigned int*)(stream_->buffer() + offset);
    *blockSize = (unsigned int)(stream_->position() - offset);
}

bool BinaryOArchive::operator()(bool &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_BOOL, _name);
	unsigned char byteValue = value ? 1 : 0;
    stream_->write(byteValue);
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(StringInterface &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_STRING, _name);
    stream_->write(value.get(), strlen(value.get()));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(WStringInterface &value, const char *_name, const char* label)
{
	openNode(BINARY_NODE_WSTRING, _name);
	stream_->write(value.get(), sizeof(wchar_t) * wcslen(value.get()));
	closeNode();
	return true;
}

bool BinaryOArchive::operator()(float &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_FLOAT, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(double &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_DOUBLE, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(short &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_INT16, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(signed char &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_SBYTE, _name);
    stream_->write((char)value);
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(unsigned char &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_BYTE, _name);
    stream_->write((char)value);
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(char &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_SBYTE, _name);
    stream_->write(value);
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(unsigned short &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_UINT16, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}



bool BinaryOArchive::operator()(int &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_INT32, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(unsigned int &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_UINT32, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(long long &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_INT64, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(unsigned long long &value, const char *_name, const char* label)
{
    openNode(BINARY_NODE_UINT64, _name);
    stream_->write((const char*)&value, sizeof(value));
    closeNode();
    return true;
}

bool BinaryOArchive::operator()(const Serializer &ser, const char *_name, const char* label)
{
    openStruct(_name, ser.type().name());

    ser(*this);

    closeStruct();
    return true;
}

bool BinaryOArchive::operator()(ContainerInterface &ser, const char *_name, const char* label)
{
    int size = int(ser.size());
    TypeID type = ser.elementType();
    openContainer(_name, size, type.name());

    if (size > 0)
        do {
            ser(*this, "", "");
        } while (ser.next());

    closeContainer();
    return true;
}


bool BinaryOArchive::operator()(PointerInterface &_ptr, const char *_name, const char* label)
{
    openNode(BINARY_NODE_POINTER, _name);
	TypeID derived = _ptr.type();
	const TypeDescription* desc = 0;
	if (derived) {
		desc = _ptr.factory()->descriptionByType(derived);
		YASLI_ASSERT(desc != 0 && "Writing unregistered class. Use YASLI_CLASS macro for registration.");
	}

    TypeID baseType = _ptr.baseType();
    write(baseType.name());
	write(desc ? desc->name() : "");

	if(_ptr.get())
		_ptr.serializer()(*this);

    closeNode();
    return true;
}

}
