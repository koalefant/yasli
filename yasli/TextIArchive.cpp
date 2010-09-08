#include "StdAfx.h"
#include <cstdio>
#include <algorithm>
#include "yasli/TextIArchive.h"
#include "yasli/MemoryReader.h"
#include "yasli/MemoryWriter.h"
#include "yasli/StringUtil.h"
#include "yasli/Unicode.h"

// #define DEBUG_TEXTIARCHIVE

namespace yasli{

TextIArchive::TextIArchive()
: Archive(true)
{
}

TextIArchive::~TextIArchive()
{
	stack_.clear();
	reader_.reset();
}

bool TextIArchive::open(const char* buffer, size_t length, bool free)
{
	if(buffer)
		reader_.reset(new MemoryReader(buffer, length, free));

	token_ = Token(reader_->begin(), reader_->begin());
	stack_.clear();

	stack_.push_back(Level());
	readToken();
	putToken();
	stack_.back().start = token_.end;
	return true;
}


bool TextIArchive::load(const char* filename)
{
	if(std::FILE* file = ::yasli::fopen(filename, "rb"))
	{
		std::fseek(file, 0, SEEK_END);
		long fileSize = std::ftell(file);
		std::fseek(file, 0, SEEK_SET);

		void* buffer = 0;
		if(fileSize > 0){
			buffer = std::malloc(fileSize + 1);
			ASSERT(buffer);
			memset(buffer, 0, fileSize + 1);
			long elementsRead = std::fread(buffer, fileSize, 1, file);
			ASSERT(((char*)(buffer))[fileSize] == '\0');
			if(elementsRead != 1){
				return false;
			}
		}
		std::fclose(file);

		filename_ = filename;
		return open((char*)buffer, fileSize, true);
	}
	else{
		return false;
	}
}

Token TextIArchive::readToken()
{
	token_ = tokenizer_(token_.end);
#ifdef DEBUG_TEXTIARCHIVE
	std::cout << " ~ read token '" << token_.str() << "' at " << token_.start - reader_->begin() << std::endl ;
	if(token_){
		ASSERT(token_.start < reader_->end());
		ASSERT(token_.start[0] != '\001');
	}
#endif
	return token_;
}

void TextIArchive::putToken()
{
#ifdef DEBUG_TEXTIARCHIVE
    std::cout << " putToken: \'" << token_.str() << "\'" << std::endl;
#endif
    token_ = Token(token_.start, token_.start);
}

int TextIArchive::line(const char* position) const
{
	return std::count(reader_->begin(), position, '\n') + 1;
}

bool TextIArchive::isName(Token token) const
{
	if(!token)
		return false;
	char firstChar = token.start[0];
	ASSERT(firstChar != '\0');
	ASSERT(firstChar != ' ');
	ASSERT(firstChar != '\n');
    if((firstChar >= 'a' && firstChar <= 'z') ||
       (firstChar >= 'A' && firstChar <= 'Z'))
	{
		size_t len = token_.length();
		if(len == 4)
		{
			if((token_ == "true") ||
			   (token_ == "True") ||
			   (token_ == "TRUE"))
				return false;
		}
		else if(len == 5)
		{
			if((token_ == "false") ||
			   (token_ == "False") ||
			   (token_ == "FALSE"))
				return false;
		}
		return true;
	}
    return false;
}


void TextIArchive::expect(char token)
{
    if(token_ != token){
        MemoryWriter msg;
        msg << "Error parsing file, expected '=' at line " << line(token_.start);
		ASSERT_STR(0, msg.c_str());
    }
}

void TextIArchive::skipBlock()
{
#ifdef DEBUG_TEXTIARCHIVE
    std::cout << "Skipping block from " << token_.end - reader_->start << " ..." << std::endl;
#endif
    if(openBracket() || openContainerBracket())
        closeBracket(); // Skipping entire block
    else
        readToken(); // Skipping value
#ifdef DEBUG_TEXTIARCHIVE
    std::cout << "...till " << token_.end - reader_->start << std::endl;
#endif
}

bool TextIArchive::findName(const char* name)
{

#ifdef DEBUG_TEXTIARCHIVE
    std::cout << " * finding name '" << name << "'" << std::endl;
    std::cout << "   started at byte " << int(token_.start - reader_->start) << std::endl;
#endif
    ASSERT(!stack_.empty());
    const char* start = 0;
    const char* blockBegin = stack_.back().start;
	if(*blockBegin == '\0')
		return false;

    readToken();
    if(!token_){
	    start = blockBegin;
        token_.set(blockBegin, blockBegin);
		readToken();
	}

    if(name[0] == '\0'){
        if(isName(token_)){
#ifdef DEBUG_TEXTIARCHIVE
            std::cout << "Token: '" << token_.str() << "'" << std::endl;
#endif

            start = token_.start;
            readToken();
            expect('=');
            skipBlock();
        }
        else{
			if(token_ == ']' || token_ == '}'){ // CONVERSION
#ifdef DEBUG_TEXTIARCHIVE
                std::cout << "Got close bracket..." << std::endl;
#endif
                putToken();
                return false;
            }
            else{
#ifdef DEBUG_TEXTIARCHIVE
                std::cout << "Got unnamed value: '" << token_.str() << "'" << std::endl;
#endif
                putToken();
                return true;
            }
        }
    }
    else{
        if(isName(token_)){
#ifdef DEBUG_TEXTIARCHIVE
            std::cout << "Seems to be a name '" << token_.str() << "'" << std::endl ;
#endif
            if(token_ == name){
                readToken();
                expect('=');
#ifdef DEBUG_TEXTIARCHIVE
                std::cout << "Got one" << std::endl;
#endif
                return true;
            }
            else{
                start = token_.start;

                readToken();
                expect('=');
                skipBlock();
            }
        }
        else{
            start = token_.start;
			if(token_ == ']' || token_ == '}') // CONVERSION
                token_ = Token(blockBegin, blockBegin);
            else{
                putToken();
                skipBlock();
            }
        }
    }

    while(true){
        readToken();
		if(!token_){
			token_.set(blockBegin, blockBegin);
			continue;
		}
            //return false; // Reached end of file while searching for name
#ifdef DEBUG_TEXTIARCHIVE
        std::cout << "'" << token_.str() << "'" << std::endl;
        std::cout << "Checking for loop: " << token_.start - reader_->start << " and " << start - reader_->begin() << std::endl;
#endif
        ASSERT(start);
        if(token_.start == start){
            putToken();
#ifdef DEBUG_TEXTIARCHIVE
            std::cout << "unable to find..." << std::endl;
#endif
            return false; // Reached a full circle: unable to find name
        }

		if(token_ == '}' || token_ == ']'){ // CONVERSION
#ifdef DEBUG_TEXTIARCHIVE
            std::cout << "Going to begin of block, from " << token_.start - reader_->begin();
#endif
            token_ = Token(blockBegin, blockBegin);
#ifdef DEBUG_TEXTIARCHIVE
            std::cout << " to " << token_.start - reader_->begin() << std::endl;
#endif
            continue; // Reached '}' or ']' while searching for name, continue from begin of block
        }

        if(name[0] == '\0'){
            if(isName(token_)){
                readToken();
                if(!token_)
                    return false; // Reached end of file while searching for name
                expect('=');
                skipBlock();
            }
            else{
                putToken(); // Not a name - put it back
                return true;
            }
        }
        else{
            if(isName(token_)){
                Token nameToken = token_; // token seems to be a name
                readToken();
                expect('=');
                if(nameToken == name)
                    return true; // Success! we found our name
                else
                    skipBlock();
            }
            else{
                putToken();
                skipBlock();
            }
        }

    }

    return false;
}

bool TextIArchive::openBracket()
{
	Token tok = readToken();
	if(tok != '{'){
        putToken();
        return false;
    }
    return true;
}

bool TextIArchive::closeBracket()
{
    int relativeLevel = 0;
    while(true){
        readToken();

        if(!token_){
            MemoryWriter msg;
            ASSERT(!stack_.empty());
            const char* start = stack_.back().start;
            msg << filename_.c_str() << ": " << line(start) << " line";
            msg << ": End of file while no matching bracket found";
			ASSERT_STR(0, msg.c_str());
			return false;
        }
		else if(token_ == '}' || token_ == ']'){ // CONVERSION
            if(relativeLevel == 0)
				return token_ == '}';
            else
                --relativeLevel;
        }
		else if(token_ == '{' || token_ == '['){ // CONVERSION
            ++relativeLevel;
        }
    }
    return false;
}

bool TextIArchive::openContainerBracket()
{
    if(readToken() != '['){
        putToken();
        return false;
    }
    return true;
}

bool TextIArchive::closeContainerBracket()
{
    readToken();
    if(token_ == ']'){
#ifdef DEBUG_TEXTIARCHIVE
        std::cout << "closeContainerBracket(): ok" << std::endl;
#endif
        return true;
    }
    else{
#ifdef DEBUG_TEXTIARCHIVE
        std::cout << "closeContainerBracket(): failed ('" << token_.str() << "')" << std::endl;
#endif
        putToken();
        return false;
    }
}

bool TextIArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
    if(findName(name)){
        if(openBracket()){
            stack_.push_back(Level());
            stack_.back().start = token_.end;
            ser(*this);
            ASSERT(!stack_.empty());
            stack_.pop_back();
            bool closed = closeBracket();
            ASSERT(closed);
            return true;
        }
    }
    return false;
}

bool TextIArchive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
    if(findName(name)){
        if(openContainerBracket()){
            stack_.push_back(Level());
            stack_.back().start = token_.end;

            std::size_t size = ser.size();
            std::size_t index = 0;

            while(true){
                readToken();
				if(token_ == '}')
				{
					ASSERT(0 && "Syntax error: closing container with '}'")
					return false;
				}
                if(token_ == ']')
                    break;
                else if(!token_)
				{
                    ASSERT(0 && "Reached end of file while reading container!");
					return false;
				}
                putToken();
                if(index == size)
                    size = index + 1;
                if(index < size){
                    ser(*this, "", "");
                }
                else{
                    skipBlock();
                }
                ser.next();
				++index;
            }
            if(size > index)
                ser.resize(index);

            ASSERT(!stack_.empty());
            stack_.pop_back();
            return true;
        }
    }
    return false;
}

void TextIArchive::checkValueToken()
{
    if(!token_){
        ASSERT(!stack_.empty());
        MemoryWriter msg;
        const char* start = stack_.back().start;
        msg << filename_.c_str() << ": " << line(start) << " line";
        msg << ": End of file while reading element's value";
        ASSERT_STR(0, msg.c_str());
    }
}

bool TextIArchive::checkStringValueToken()
{
    if(!token_){
        return false;
        MemoryWriter msg;
        const char* start = stack_.back().start;
        msg << filename_.c_str() << ": " << line(start) << " line";
        msg << ": End of file while reading element's value";
        ASSERT_STR(0, msg.c_str());
		return false;
    }
    if(token_.start[0] != '"' || token_.end[-1] != '"'){
        return false;
        MemoryWriter msg;
        const char* start = stack_.back().start;
        msg << filename_.c_str() << ": " << line(start) << " line";
        msg << ": Expected string";
        ASSERT_STR(0, msg.c_str());
		return false;
    }
    return true;
}

bool TextIArchive::operator()(int& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        value = strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool TextIArchive::operator()(unsigned int& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        value = strtoul(token_.start, 0, 10);
        return true;
    }
    return false;
}


bool TextIArchive::operator()(short& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        value = (short)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool TextIArchive::operator()(unsigned short& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        value = (unsigned short)strtoul(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool TextIArchive::operator()(__int64& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
#ifdef WIN32
		value = _strtoui64(token_.start, 0, 10);
#else
#endif
        return true;
    }
    return false;
}

bool TextIArchive::operator()(float& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
#ifdef _MSC_VER
        value = float(std::atof(token_.str().c_str()));
#else
        value = std::strtof(token_.start, 0);
#endif
        return true;
    }
    return false;
}

bool TextIArchive::operator()(double& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
#ifdef _MSC_VER
        value = std::atof(token_.str().c_str());
#else
        value = std::strtof(token_.start, 0);
#endif
        return true;
    }
    return false;
}

bool TextIArchive::operator()(std::string& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        if(checkStringValueToken())
			unescapeString(value, token_.start + 1, token_.end - 1);
		else
			return false;
        return true;
    }
    return false;
}

bool TextIArchive::operator()(bool& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        if(token_ == "true" || token_ == "True" || token_ == "TRUE")
            value = true;
        else
            value = false;
        return true;
    }
    return false;
}

bool TextIArchive::operator()(signed char& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        value = (signed char)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool TextIArchive::operator()(unsigned char& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        value = (unsigned char)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool TextIArchive::operator()(char& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        value = (char)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool TextIArchive::operator()(StringListStaticValue& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        if(checkStringValueToken()){
            Token tok(token_.start + 1, token_.end - 1);
            ASSERT(token_.start < token_.end);
            ASSERT(tok.start < tok.end);
            std::string str = tok.str();
#ifdef DEBUG_TEXTIARCHIVE
            std::cout << "Reading '" << str << "'" << std::endl;
#endif

            int index = value.stringList().find(str.c_str());
            if(index != StringListStatic::npos){
                value = index;
                return true;
            }
            else{
                warning("Unable to read StringListStaticValue, no such value");
                return false;
            }

        }
        else
            return false;
    }
    return false;
}

}
