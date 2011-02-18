#pragma once

#include "Archive.h"
#include "MemoryWriter.h" 

namespace yasli{

class MemoryWriter;
enum BinaryNode;

class BinaryOArchive : public Archive{
public:

  BinaryOArchive(bool pretendToBeEdit = false);
  ~BinaryOArchive() {}

  void clear();
  size_t length() const;
  const char* buffer();
  bool save(const char* fileName);

  bool operator()(bool &value, const char *name, const char* label);
  bool operator()(std::string &value, const char *name, const char* label);
  bool operator()(float &value, const char *name, const char* label);
  bool operator()(double &value, const char *name, const char* label);
  bool operator()(int &value, const char *name, const char* label);
  bool operator()(unsigned int &value, const char *name, const char* label);
  bool operator()(short &value, const char *name, const char* label);
  bool operator()(unsigned short &value, const char *name, const char* label);
  bool operator()(__int64 &value, const char *name, const char* label);

  bool operator()(signed char &value, const char *name, const char* label);
  bool operator()(unsigned char &value, const char *name, const char* label);
  bool operator()(char &value, const char *name, const char* label);

  bool operator()(const Serializer &ser, const char *name, const char* label);
  bool operator()(ContainerSerializationInterface &ser, const char *name, const char* label);
  bool operator()(const PointerSerializationInterface &ptr, const char *name, const char* label);

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
  SharedPtr<MemoryWriter> stream_;
};

}