/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "Archive.h"
#include "MemoryWriter.h" 
#include "BinaryNode.h"
#include "Pointers.h"

namespace yasli{

class MemoryWriter;

class BinaryOArchive : public Archive{
public:

  BinaryOArchive(bool pretendToBeEdit = false);
  ~BinaryOArchive() {}

  void clear();
  size_t length() const;
  const char* buffer();
  bool save(const char* fileName);

  bool operator()(bool &value, const char *name, const char* label);
  bool operator()(StringInterface &value, const char *name, const char* label);
  bool operator()(WStringInterface &value, const char *name, const char* label);
  bool operator()(float &value, const char *name, const char* label);
  bool operator()(double &value, const char *name, const char* label);
  bool operator()(int &value, const char *name, const char* label);
  bool operator()(unsigned int &value, const char *name, const char* label);
  bool operator()(short &value, const char *name, const char* label);
  bool operator()(unsigned short &value, const char *name, const char* label);
  bool operator()(long long &value, const char *name, const char* label);
  bool operator()(unsigned long long &value, const char *name, const char* label);

  bool operator()(signed char &value, const char *name, const char* label);
  bool operator()(unsigned char &value, const char *name, const char* label);
  bool operator()(char &value, const char *name, const char* label);

  bool operator()(const Serializer &ser, const char *name, const char* label);
  bool operator()(ContainerInterface &ser, const char *name, const char* label);
  bool operator()(PointerInterface &ptr, const char *name, const char* label);

  using Archive::operator();

private:
  void openContainer(const char* name, int size, const char* typeName);
  void closeContainer();
  void openStruct(const char *name, const char *typeName);
  void closeStruct();
  void openNode(BinaryNode type, const char *name);
  void closeNode();
  void write(const char *str);

  std::vector<unsigned int> blockSizeOffsets_;
  std::auto_ptr<MemoryWriter> stream_;
};

}
