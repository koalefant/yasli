#pragma once

#include "yasli/Archive.h"
#include "yasli/Tokenizer.h"
#include "yasli/Pointers.h"

#include "yasli/MemoryReader.h"

class YASLI_API TextIArchive : public Archive{
public:
    TextIArchive();
    ~TextIArchive();

    bool load(const char* filename);
	bool open(char* buffer, size_t length, bool free = false);

    // virtuals:
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

    bool operator()(StringListStaticValue& value, const char* name = "", const char* label = 0);

	using Archive::operator();
private:
    bool findName(const char* name);
    bool openBracket();
    bool closeBracket();

    bool openContainerBracket();
    bool closeContainerBracket();

    void checkValueToken();
    bool checkStringValueToken();
    Token readToken();
    void putToken();
    int line(const char* position) const; 
    bool isName(Token token) const;

    void expect(char token);
    void skipBlock();

    struct Level{
        const char* start;
        const char* firstToken;
    };
    typedef std::vector<Level> Stack;
    Stack stack_;

    Tokenizer tokenizer_;
    AutoPtr<MemoryReader> reader_;
    Token token_;
    std::string filename_;
};
