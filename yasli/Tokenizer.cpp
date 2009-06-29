#include "StdAfx.h"
#include "yasli/Tokenizer.h"
#include <iostream>

// #define DEBUG_TOKENIZER

namespace yasli{

Tokenizer::Tokenizer(const char* spaces, const char* quotes, const char* comments)
: spaces_(spaces)
, comments_(comments)
, quotes_(quotes)
{
    ASSERT(quotes_.length() % 2 == 0);
    ASSERT(strlen(spaces_.c_str()) > 0);
    //ASSERT(strlen(comments_.c_str()) > 0);
    //ASSERT(strlen(quotes_.c_str()) > 0);
}

inline bool Tokenizer::isSpace(char c) const
{
    const char* ptr = spaces_.c_str();
    while(*ptr != '\0'){
        if(*ptr == c)
            return true;
        ++ptr;
    }
    return false;
}

inline bool Tokenizer::isComment(char c) const
{
    const char* ptr = comments_.c_str();
    while(*ptr != '\0'){
        if(*ptr == c)
            return true;
        ++ptr;
    }
    return false;
}

inline bool Tokenizer::isQuoteOpen(int& quoteIndex, char c) const
{
    quoteIndex = 0;
    const char* ptr = quotes_.c_str();
    while(*ptr){
        if(*ptr == c)
            return true;
        ptr += 2;
        ++quoteIndex;
    }
    return false;
}

inline bool Tokenizer::isQuote(char c) const
{
    const char* ptr = quotes_.c_str();
    while(*ptr){
        if(*ptr == c)
            return true;
        ++ptr;
    }
    return false;
}

inline bool Tokenizer::isQuoteClose(int quoteIndex, char c) const
{
    const char* ptr = quotes_.c_str();
    ptr += 2 * quoteIndex;;
    if(ptr[1] == c)
        return true;
    return false;
}

inline bool Tokenizer::isWordPart(char c) const
{
    return !isSpace(c) && !isQuote(c) && !isComment(c);
}

Token Tokenizer::operator()(const char* ptr) const
{
    while(isSpace(*ptr))
        ++ptr;
    Token cur(ptr, ptr);
    while(!cur && *ptr != '\0'){
        while(isComment(*cur.end_)){
#ifdef DEBUG_TOKENIZER
            const char* commentStart = ptr;
#endif
            while(*cur.end_ && *cur.end_ != '\n')
                ++cur.end_;
            while(isSpace(*cur.end_))
                ++cur.end_;
#ifdef DEBUG_TOKENIZER
            std::cout << "Got comment: '" << std::string(commentStart, cur.end_) << "'" << std::endl;
#endif
            cur.start_ = cur.end_;
        }
        ASSERT(!isSpace(*cur.end_));
        int quoteIndex = 0;
        if(isQuoteOpen(quoteIndex, *cur.end_)){
            ++cur.end_;
            while(*cur.end_){ 
                if(*cur.end_ == '\\'){
                    ++cur.end_;
                    if(*cur.end_ ){
                        if(*cur.end_ != 'x' && *cur.end_ != 'X')
                            ++cur.end_;
                        else{
                            ++cur.end_;
                            if(*cur.end_)
                                ++cur.end_;
                        }
                    }
                }
                if(isQuoteClose(quoteIndex, *cur.end_)){
                    ++cur.end_;
#ifdef DEBUG_TOKENIZER
                    std::cout << "Tokenizer result: " << cur.str() << std::endl;
#endif
                    return cur;
                }
                else
                    ++cur.end_;
            }
        }                                
        else{          
            //ASSERT(*cur.end_);
            if(!*cur.end_)
                return cur;

#ifdef DEBUG_TOKENIZER
            char twoChars[] = { *cur.end_, '\0' };
            std::cout << twoChars << std::endl;
            ASSERT(isWordPart(*cur.end_));
#endif
            while(*cur.end_ != '\0' && isWordPart(*cur.end_))
                ++cur.end_;               
#ifdef DEBUG_TOKENIZER
            std::cout << "Tokenizer result: " << cur.str() << std::endl;
#endif
            return cur;
        }
    }
#ifdef DEBUG_TOKENIZER
    std::cout << "Tokenizer result: " << cur.str() << std::endl;
#endif
    return cur;
}

}
