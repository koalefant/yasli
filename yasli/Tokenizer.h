#ifndef __TOKENIZER_H_INCLUDED__
#define __TOKENIZER_H_INCLUDED__

#include <cstring>
#include <string>
#include "API.h"

class Token{
    friend class Tokenizer;
public:
    Token(const char* _start = 0, const char* _end = 0)
    : start_(_start)
    , end_(_end)
    {}

	void set(const char* start, const char* end){
		start_ = start;
		end_ = end;
	}

    std::size_t length() const{ return end_ - start_; }

    bool operator==(const std::string& rhs) const{
        return operator==(rhs.c_str());
    }


    bool operator==(const char* text) const{
        if(std::strlen(text) == length())
            return std::strncmp(text, start_, length()) == 0;
        else
            return false;
    }
    bool operator!=(const char* text) const{
        if(std::strlen(text) == length())
            return std::strncmp(text, start_, length()) != 0;
        else
            return true;
    }
    bool operator==(char c) const{
        return length() == 1 && *start_ == c;
    }
    bool operator!=(char c) const{
        return length() != 1 || *start_ != c;
    }

    operator bool() const{
        return start_ != end_;
    }

    std::string str() const{ return std::string(start_, end_);  }

    const char* begin() const{ return start_; }
    const char* end() const{ return end_; }
private:
    const char* start_;
    const char* end_;
};

class YASLI_API Tokenizer{
public:
    Tokenizer(const char* spaces = " \t\r\n\x0D",
              const char* quotes = "\"\"",
              const char* comments = "#");

    Token operator()(const char* text) const;
private:
    inline bool isSpace(char c) const;
    inline bool isWordPart(char c) const;
    inline bool isComment(char c) const;
    inline bool isQuoteOpen(int& quoteIndex, char c) const;
    inline bool isQuoteClose(int quoteIndex, char c) const;
    inline bool isQuote(char c) const;

    std::string spaces_;
    std::string comments_;
    std::string quotes_;
};

#endif
