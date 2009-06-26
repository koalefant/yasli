#pragma once
#include "Archive.h"

#include "BinaryNode.h"
#include "Tokenizer.h" // for Token

class BinaryIArchive : public Archive{
public:

  BinaryIArchive();
  ~BinaryIArchive();

  bool load(const char* fileName);
  bool open(const char* buffer, size_t length); // does copy of buffer
  void close();

  const char* pull();
  bool operator()(bool& value, const char* name = "", const char* label = 0);
  bool operator()(std::string& value, const char* name = "", const char* label = 0);
  bool operator()(float& value, const char* name = "", const char* label = 0);
  bool operator()(double& value, const char* name = "", const char* label = 0);
  bool operator()(short& value, const char* name = "", const char* label = 0);
  bool operator()(unsigned short& value, const char* name = "", const char* label = 0);
  bool operator()(int& value, const char* name = "", const char* label = 0);
  bool operator()(unsigned int& value, const char* name = "", const char* label = 0);
  bool operator()(__int64& value, const char* name = "", const char* label = 0);

  bool operator()(signed char& value, const char* name = "", const char* label = 0);
  bool operator()(unsigned char& value, const char* name = "", const char* label = 0);
  bool operator()(char& value, const char* name = "", const char* label = 0);

  bool operator()(const Serializer& ser, const char* name = "", const char* label = 0);
  bool operator()(ContainerSerializationInterface& ser, const char* name = "", const char* label = 0);
  bool operator()(const PointerSerializationInterface& ptr, const char* name = "", const char* label = 0);
  // ^^^

  using Archive::operator();
private:
  Token pullNode(BinaryNode* type);
  void closeNode();
  bool findNode(BinaryNode _type, const Token &_name, const char** start, const char** end);
  bool findNode(BinaryNode _type, const char *_name);
  size_t readNodeHeader(BinaryNode* _type, Token* name);
  void closeContainer();
  void closeStruct();
  bool openContainer(const char *_name, Token *_typeName, size_t *_size);
  bool openStruct(const char *_name, Token* typeName);
  bool openPointer(const char *_name, Token *_baseType, Token *_type);
  void closePointer();


  struct Block{
    Block(const char *_start, const char *_innerStart, const char *_end)
    : start( _start )
    , innerStart( _innerStart )
    , end( _end )
    {
    }
    const char *start;
    const char *innerStart;
    const char *end;
  };
  std::vector<Block> blocks_;

  template<class T>
  bool read(T* value)
  {
    return read((char*)value, sizeof(T));
  }

  bool read(Token* str);
  bool read(char* data, int size);
  bool readUnsafe(char* data, int size);

  std::string pulledName_;
  const char* loadedData_;
  const char* pos_;
  const char* buffer_;
  const char* end_;
  const char* pullPosition_;
  size_t size_;
};
