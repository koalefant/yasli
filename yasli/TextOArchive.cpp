#include "StdAfx.h"
#include "yasli/TextOArchive.h"

#include "yasli/StringUtil.h"
#include "yasli/MemoryWriter.h"
#include "yasli/Unicode.h"

namespace yasli{

static const int TAB_WIDTH = 2;

TextOArchive::TextOArchive(int textWidth, const char* header)
: Archive(false)
, header_(header)
, textWidth_(textWidth)
{
    buffer_ = new MemoryWriter(1024, true);
    if(header_)
        (*buffer_) << header_;

    ASSERT(stack_.empty());
    stack_.push_back(Level(false, 0, 0));
}

TextOArchive::~TextOArchive()
{
    close();
}

bool TextOArchive::save(const char* fileName)
{
    ESCAPE(fileName && strlen(fileName) > 0, return false);
    ESCAPE(stack_.size() == 1, return false);
    ESCAPE(buffer_, return false);
    ESCAPE(buffer_->position() <= buffer_->size(), return false);
    stack_.pop_back();
    if(FILE* file = ::yasli::fopen(fileName, "wb")){
        if(fwrite(buffer_->c_str(), 1, buffer_->position(), file) != buffer_->position()){
            fclose(file);
            return false;
        }
        fclose(file);
        return true;
    }
    else{
        return false;
    }
}

const char* TextOArchive::c_str() const
{
    return buffer_->c_str();
}

size_t TextOArchive::length() const
{
	return buffer_->position();
}

void TextOArchive::openBracket()
{
	*buffer_ << "{\n";
}

void TextOArchive::closeBracket()
{
	*buffer_ << "}\n";
}

void TextOArchive::openContainerBracket()
{
    *buffer_ << "[\n";
}

void TextOArchive::closeContainerBracket()
{
    *buffer_ << "]\n";
}

void TextOArchive::placeName(const char* name)
{
    if(name[0] != '\0'){
        *buffer_ << name;
        *buffer_ << " = ";
    }
}

void TextOArchive::placeIndent()
{
    int count = stack_.size() - 1;
    stack_.back().indentCount += count/* * TAB_WIDTH*/;
    for(int i = 0; i < count; ++i)
        *buffer_ << "\t";
}

bool TextOArchive::operator()(bool& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    *buffer_ << (value ? "true" : "false");
    *buffer_ << "\n";
    return true;
}


bool TextOArchive::operator()(std::string& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << "\""; 
    const char* str = value.c_str();
    escapeString(*buffer_, str, str + value.size());
    (*buffer_) << "\"\n";
    return true;
}

bool TextOArchive::operator()(float& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(double& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(int& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(unsigned int& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(short& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(unsigned short& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(__int64& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(unsigned char& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(signed char& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(char& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\n";
    return true;
}

bool TextOArchive::operator()(StringListStaticValue& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << "\"" << value.c_str() << "\"\n";
    return true;
}

bool TextOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    std::size_t position = buffer_->position();
    openBracket();
    stack_.push_back(Level(false, position, strlen(name) + 2 * (name[0] & 1) + (stack_.size() - 1) * TAB_WIDTH + 2));

    ASSERT(ser);
    ser(*this);

    bool joined = joinLinesIfPossible();
    stack_.pop_back();
    if(!joined)
        placeIndent();
    //else
    //    *buffer_ << " ";
    closeBracket();
    return true;
}


bool TextOArchive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    std::size_t position = buffer_->position();
    openContainerBracket();
    stack_.push_back(Level(false, position, strlen(name) + 2 * (name[0] & 1) + (stack_.size() - 1) * TAB_WIDTH + 2));

    std::size_t size = ser.size();
    if(size > 0){
        do{
            ser(*this, "", "");
        }while(ser.next());
    }

    bool joined = joinLinesIfPossible();
    stack_.pop_back();
    if(!joined)
        placeIndent();
    closeContainerBracket();
    return true;
}

static char* joinLines(char* start, char* end)
{
    ASSERT(start <= end);
    char* next = start;
    while(next != end){
        if(*next != '\t'){
            if(*next != '\n')
                *start = *next;
            else
                *start = ' ';
            ++start;
        }
        ++next;
    }
    return start;
}

bool TextOArchive::joinLinesIfPossible()
{
    ASSERT(!stack_.empty());
    std::size_t startPosition = stack_.back().startPosition;
    ASSERT(startPosition < buffer_->size());
    int indentCount = stack_.back().indentCount;
    //ASSERT(startPosition >= indentCount);
    if(buffer_->position() - startPosition - indentCount < std::size_t(textWidth_)){
        char* buffer = buffer_->buffer();
        char* start = buffer + startPosition;
        char* end = buffer + buffer_->position();
        end = joinLines(start, end);
        std::size_t newPosition = end - buffer;
        ASSERT(newPosition <= buffer_->position());
        buffer_->setPosition(newPosition);
        return true;
    }
    return false;
}

}