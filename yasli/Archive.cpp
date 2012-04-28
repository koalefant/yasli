/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/Archive.h"
#include <string>

#include "yasli/TypesFactory.h"

namespace yasli{

static char* writeUtf16ToUtf8(char* s, unsigned int ch)
{
  const unsigned char byteMark = 0x80;
  const unsigned char byteMask = 0xBF;

  size_t len;

  if (ch < 0x80)
    len = 1;
  else if (ch < 0x800)
    len = 2;
  else if (ch < 0x10000)
    len = 3;
  else if (ch < 0x200000)
    len = 4;
  else
    return s;

  s += len;

  const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
  switch(len)
  {
  case 4:
  *--s = (char)((ch | byteMark) & byteMask); 
  ch >>= 6;
  case 3:
  *--s = (char)((ch | byteMark) & byteMask); 
  ch >>= 6;
  case 2:
  *--s = (char)((ch | byteMark) & byteMask); 
  ch >>= 6;
  case 1:
  *--s = (char)(ch | firstByteMark[len]);
  }

  return s + len;
}

static const char* readUtf16FromUtf8(unsigned int* ch, const char* s)
{
  const unsigned char byteMark = 0x80;
  const unsigned char byteMaskRead = 0x3F;

  const unsigned char* str = (const unsigned char*)s;

  size_t len;
  if(*str < byteMark)
  {
    *ch = *str;
    return s + 1;
  }
  else if(*str < 0xC0)
  {
    *ch = ' ';
    return s + 1;
  }
  else if(*str < 0xE0)
    len = 2;
  else if(*str < 0xF0)
    len = 3;
  else if(*str < 0xF8)
    len = 4;
  else if(*str < 0xFC)
    len = 5;
  else{
    *ch = ' ';
    return s + 1;
  }

  const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
  *ch = (*str++ & ~firstByteMark[len]);

  switch(len) 
  {
  case 5:
    (*ch) <<= 6;
    (*ch) += (*str++ & byteMaskRead);
  case 4:
    (*ch) <<= 6;
    (*ch) += (*str++ & byteMaskRead);
  case 3:
    (*ch) <<= 6;
    (*ch) += (*str++ & byteMaskRead);
  case 2:
    (*ch) <<= 6;
    (*ch) += (*str++ & byteMaskRead);
  }
  
  return (const char*)str;
}

size_t utf16inUtf8Len(const wchar_t* p)
{
  size_t result = 0;

  for(; *p; ++p)
  {
    unsigned int ch = *p;

    if (ch < 0x80)
      result += 1;
    else if (ch < 0x800)
      result += 2;
    else if (ch < 0x10000)
      result += 3;
    else if (ch < 0x200000)
      result += 4;
  }

  return result;
}


static void utf16ToUtf8(std::string* out, const wchar_t* in)
{
  out->clear();
  out->reserve(utf16inUtf8Len(in));

  for(; *in; ++in)
  {
    char buf[6];
    out->append(buf, writeUtf16ToUtf8(buf, *in));
  }
}

static size_t utf8InUtf16Len(const char* p)
{
  size_t result = 0;

  for(; *p; ++p)
  {
    unsigned char ch = (unsigned char)(*p);

    if(ch < 0x80 || (ch >= 0xC0 && ch < 0xFC))
      ++result;
  }

  return result;
}

static void utf8ToUtf16(std::wstring* out, const char* in)
{
  out->clear();
  out->reserve(utf8InUtf16Len(in));

  for (; *in;)
  {
    unsigned int character;
    in = readUtf16FromUtf8(&character, in);
    (*out) += (wchar_t)character;
  }
}

// ---------------------------------------------------------------------------

void Archive::warning(const char* message)
{
	// TODO: provide messaging-interface
}

bool Archive::operator()(std::wstring& value, const char* name, const char* label)
{
  std::string str;
  if(isOutput())
    utf16ToUtf8(&str, value.c_str());
  if (!(*this)(str, name, label))
    return false;
  if(isInput())
    utf8ToUtf16(&value, str.c_str());
	return true;
}

bool Archive::operator()(ContainerSerializationInterface& ser, const char* name, const char* label)
{
	return false;
}

bool Archive::operator()(const PointerSerializationInterface& ptr, const char* name, const char* label)
{
	return (*this)(Serializer(const_cast<PointerSerializationInterface&>(ptr)), name, label);
}

void Archive::notImplemented()
{
    ASSERT(0 && "Not implemented!");
}
// ---------------------------------------------------------------------------

bool Serializer::operator()(Archive& ar) const{
	ESCAPE(serializeFunc_ && object_, return false);
    return serializeFunc_(object_, ar);
}
bool Serializer::operator()(Archive& ar, const char* name, const char* label) const{
    return ar(*this, name, label);
}

// ---------------------------------------------------------------------------

void PointerSerializationInterface::serialize(Archive& ar) const
{
    TypeID baseTypeID = baseType();
    TypeID oldTypeID = type();

    if(ar.isOutput()){
        if(oldTypeID){
            if(ar(oldTypeID, "")){
                ar(serializer(), "");
            }
            else
                ar.warning("Unable to write typeID!");
        }
    }
    else{
        TypeID typeID;
        if(!ar(typeID, "")){
            if(oldTypeID){
                create(TypeID()); // 0
            }
            return;
        }

        if(oldTypeID && (!typeID || (typeID != oldTypeID)))
            create(TypeID()); // 0

        if(typeID){
            if(!get())
                create(typeID);
            ar(serializer(), "");
        }
    }	
}

}

namespace std{
YASLI_TYPE_NAME(string, "string")
}
