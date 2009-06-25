#pragma once

#include "Archive.h"
#include "Tokenizer.h" // for Token

class MemoryWriter;
enum BinaryNode;
class BinaryOArchive : public Archive{
public:

  BinaryOArchive(bool writeTypeInfo = false);
  ~BinaryOArchive() {}

  void clear();
  size_t length() const;
  const char* buffer();

  bool operator()(bool &value, const char *name);
  bool operator()(std::string &value, const char *name);
  bool operator()(float &value, const char *name);
  bool operator()(double &value, const char *name);
  bool operator()(int &value, const char *name);
  bool operator()(unsigned int &value, const char *name);
  bool operator()(short &value, const char *name);
  bool operator()(unsigned short &value, const char *name);
  bool operator()(__int64 &value, const char *name);

  bool operator()(signed char &value, const char *name);
  bool operator()(unsigned char &value, const char *name);
  bool operator()(char &value, const char *name);

  bool operator()(const Serializer &ser, const char *name);
  bool operator()(ContainerSerializationInterface &ser, const char *name);
  bool operator()(const PointerSerializationInterface &ptr, const char *name);

  using Archive::operator();
private:
  void openContainer(const char* name, int size, const char* typeName);
  void closeContainer();
  void openStruct(const char *name, const char *typeName);
  void closeStruct();
  void openNode(BinaryNode type, const char *name);
  void closeNode();
  void write(const char *str);

  bool writeTypeInfo_;
  std::vector<unsigned int> blockSizeOffsets_;

  SharedPtr<MemoryWriter> stream_;
};
