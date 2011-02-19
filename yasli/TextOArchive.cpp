#include "StdAfx.h"
#include "yasli/TextOArchive.h"

#include "yasli/MemoryWriter.h"
#include "yasli/Unicode.h"

namespace yasli{

static const char* escapeTable[256] = {
    "\\0" /* 0x00: */,
    "\\x01" /* 0x01: */,
    "\\x02" /* 0x02: */,
    "\\x03" /* 0x03: */,
    "\\x04" /* 0x04: */,
    "\\x05" /* 0x05: */,
    "\\x06" /* 0x06: */,
    "\\x07" /* 0x07: */,
    "\\x08" /* 0x08: */,
    "\\t"   /* 0x09: \t */,
    "\\n"   /* 0x0A: \n */,
    "\\x0B" /* 0x0B: */,
    "\\x0C" /* 0x0C: */,
    "\\x0D" /* 0x0D: */,
    "\\x0E" /* 0x0E: */,
    "\\x0F" /* 0x0F: */,
    
    
    "\\x10" /* 0x10: */,
    "\\x11" /* 0x11: */,
    "\\x12" /* 0x12: */,
    "\\x13" /* 0x13: */,
    "\\x14" /* 0x14: */,
    "\\x15" /* 0x15: */,
    "\\x16" /* 0x16: */,
    "\\x17" /* 0x17: */,
    "\\x18" /* 0x18: */,
    "\\x19" /* 0x19: */,
    "\\x1A" /* 0x1A: */,
    "\\x1B" /* 0x1B: */,
    "\\x1C" /* 0x1C: */,
    "\\x1D" /* 0x1D: */,
    "\\x1E" /* 0x1E: */,
    "\\x1F" /* 0x1F: */,
    
    
    " " /* 0x20:   */,
    "!" /* 0x21: ! */,
    "\\\"" /* 0x22: " */,
    "#" /* 0x23: # */,
    "$" /* 0x24: $ */,
    "%" /* 0x25: % */,
    "&" /* 0x26: & */,
    "'" /* 0x27: ' */,
    "(" /* 0x28: ( */,
    ")" /* 0x29: ) */,
    "*" /* 0x2A: * */,
    "+" /* 0x2B: + */,
    "," /* 0x2C: , */,
    "-" /* 0x2D: - */,
    "." /* 0x2E: . */,
    "/" /* 0x2F: / */,
    
    
    "0" /* 0x30: 0 */,
    "1" /* 0x31: 1 */,
    "2" /* 0x32: 2 */,
    "3" /* 0x33: 3 */,
    "4" /* 0x34: 4 */,
    "5" /* 0x35: 5 */,
    "6" /* 0x36: 6 */,
    "7" /* 0x37: 7 */,
    "8" /* 0x38: 8 */,
    "9" /* 0x39: 9 */,
    ":" /* 0x3A: : */,
    ";" /* 0x3B: ; */,
    "<" /* 0x3C: < */,
    "=" /* 0x3D: = */,
    ">" /* 0x3E: > */,
    "?" /* 0x3F: ? */,
    
    
    "@" /* 0x40: @ */,
    "A" /* 0x41: A */,
    "B" /* 0x42: B */,
    "C" /* 0x43: C */,
    "D" /* 0x44: D */,
    "E" /* 0x45: E */,
    "F" /* 0x46: F */,
    "G" /* 0x47: G */,
    "H" /* 0x48: H */,
    "I" /* 0x49: I */,
    "J" /* 0x4A: J */,
    "K" /* 0x4B: K */,
    "L" /* 0x4C: L */,
    "M" /* 0x4D: M */,
    "N" /* 0x4E: N */,
    "O" /* 0x4F: O */,
    
    
    "P" /* 0x50: P */,
    "Q" /* 0x51: Q */,
    "R" /* 0x52: R */,
    "S" /* 0x53: S */,
    "T" /* 0x54: T */,
    "U" /* 0x55: U */,
    "V" /* 0x56: V */,
    "W" /* 0x57: W */,
    "X" /* 0x58: X */,
    "Y" /* 0x59: Y */,
    "Z" /* 0x5A: Z */,
    "[" /* 0x5B: [ */,
    "\\\\" /* 0x5C: \ */,
    "]" /* 0x5D: ] */,
    "^" /* 0x5E: ^ */,
    "_" /* 0x5F: _ */,
    
    
    "`" /* 0x60: ` */,
    "a" /* 0x61: a */,
    "b" /* 0x62: b */,
    "c" /* 0x63: c */,
    "d" /* 0x64: d */,
    "e" /* 0x65: e */,
    "f" /* 0x66: f */,
    "g" /* 0x67: g */,
    "h" /* 0x68: h */,
    "i" /* 0x69: i */,
    "j" /* 0x6A: j */,
    "k" /* 0x6B: k */,
    "l" /* 0x6C: l */,
    "m" /* 0x6D: m */,
    "n" /* 0x6E: n */,
    "o" /* 0x6F: o */,
    
    
    "p" /* 0x70: p */,
    "q" /* 0x71: q */,
    "r" /* 0x72: r */,
    "s" /* 0x73: s */,
    "t" /* 0x74: t */,
    "u" /* 0x75: u */,
    "v" /* 0x76: v */,
    "w" /* 0x77: w */,
    "x" /* 0x78: x */,
    "y" /* 0x79: y */,
    "z" /* 0x7A: z */,
    "{" /* 0x7B: { */,
    "|" /* 0x7C: | */,
    "}" /* 0x7D: } */,
    "~" /* 0x7E: ~ */,
    "\x7F" /* 0x7F: */, // for utf-8
    
    
    "\x80" /* 0x80: */,
    "\x81" /* 0x81: */,
    "\x82" /* 0x82: */,
    "\x83" /* 0x83: */,
    "\x84" /* 0x84: */,
    "\x85" /* 0x85: */,
    "\x86" /* 0x86: */,
    "\x87" /* 0x87: */,
    "\x88" /* 0x88: */,
    "\x89" /* 0x89: */,
    "\x8A" /* 0x8A: */,
    "\x8B" /* 0x8B: */,
    "\x8C" /* 0x8C: */,
    "\x8D" /* 0x8D: */,
    "\x8E" /* 0x8E: */,
    "\x8F" /* 0x8F: */,
    
    
    "\x90" /* 0x90: */,
    "\x91" /* 0x91: */,
    "\x92" /* 0x92: */,
    "\x93" /* 0x93: */,
    "\x94" /* 0x94: */,
    "\x95" /* 0x95: */,
    "\x96" /* 0x96: */,
    "\x97" /* 0x97: */,
    "\x98" /* 0x98: */,
    "\x99" /* 0x99: */,
    "\x9A" /* 0x9A: */,
    "\x9B" /* 0x9B: */,
    "\x9C" /* 0x9C: */,
    "\x9D" /* 0x9D: */,
    "\x9E" /* 0x9E: */,
    "\x9F" /* 0x9F: */,
    
    
    "\xA0" /* 0xA0: */,
    "Ў" /* 0xA1: Ў */,
    "ў" /* 0xA2: ў */,
    "Ј" /* 0xA3: Ј */,
    "¤" /* 0xA4: ¤ */,
    "Ґ" /* 0xA5: Ґ */,
    "¦" /* 0xA6: ¦ */,
    "§" /* 0xA7: § */,
    "Ё" /* 0xA8: Ё */,
    "©" /* 0xA9: © */,
    "Є" /* 0xAA: Є */,
    "«" /* 0xAB: « */,
    "¬" /* 0xAC: ¬ */,
    "­" /* 0xAD: ­ */,
    "®" /* 0xAE: ® */,
    "Ї" /* 0xAF: Ї */,
    
    
    "°" /* 0xB0: ° */,
    "±" /* 0xB1: ± */,
    "І" /* 0xB2: І */,
    "і" /* 0xB3: і */,
    "ґ" /* 0xB4: ґ */,
    "µ" /* 0xB5: µ */,
    "¶" /* 0xB6: ¶ */,
    "·" /* 0xB7: · */,
    "ё" /* 0xB8: ё */,
    "№" /* 0xB9: № */,
    "є" /* 0xBA: є */,
    "»" /* 0xBB: » */,
    "ј" /* 0xBC: ј */,
    "Ѕ" /* 0xBD: Ѕ */,
    "ѕ" /* 0xBE: ѕ */,
    "ї" /* 0xBF: ї */,
    
    
    "А" /* 0xC0: А */,
    "Б" /* 0xC1: Б */,
    "В" /* 0xC2: В */,
    "Г" /* 0xC3: Г */,
    "Д" /* 0xC4: Д */,
    "Е" /* 0xC5: Е */,
    "Ж" /* 0xC6: Ж */,
    "З" /* 0xC7: З */,
    "И" /* 0xC8: И */,
    "Й" /* 0xC9: Й */,
    "К" /* 0xCA: К */,
    "Л" /* 0xCB: Л */,
    "М" /* 0xCC: М */,
    "Н" /* 0xCD: Н */,
    "О" /* 0xCE: О */,
    "П" /* 0xCF: П */,
    
    
    "Р" /* 0xD0: Р */,
    "С" /* 0xD1: С */,
    "Т" /* 0xD2: Т */,
    "У" /* 0xD3: У */,
    "Ф" /* 0xD4: Ф */,
    "Х" /* 0xD5: Х */,
    "Ц" /* 0xD6: Ц */,
    "Ч" /* 0xD7: Ч */,
    "Ш" /* 0xD8: Ш */,
    "Щ" /* 0xD9: Щ */,
    "Ъ" /* 0xDA: Ъ */,
    "Ы" /* 0xDB: Ы */,
    "Ь" /* 0xDC: Ь */,
    "Э" /* 0xDD: Э */,
    "Ю" /* 0xDE: Ю */,
    "Я" /* 0xDF: Я */,
    
    
    "а" /* 0xE0: а */,
    "б" /* 0xE1: б */,
    "в" /* 0xE2: в */,
    "г" /* 0xE3: г */,
    "д" /* 0xE4: д */,
    "е" /* 0xE5: е */,
    "ж" /* 0xE6: ж */,
    "з" /* 0xE7: з */,
    "и" /* 0xE8: и */,
    "й" /* 0xE9: й */,
    "к" /* 0xEA: к */,
    "л" /* 0xEB: л */,
    "м" /* 0xEC: м */,
    "н" /* 0xED: н */,
    "о" /* 0xEE: о */,
    "п" /* 0xEF: п */,
    
    
    "р" /* 0xF0: р */,
    "с" /* 0xF1: с */,
    "т" /* 0xF2: т */,
    "у" /* 0xF3: у */,
    "ф" /* 0xF4: ф */,
    "х" /* 0xF5: х */,
    "ц" /* 0xF6: ц */,
    "ч" /* 0xF7: ч */,
    "ш" /* 0xF8: ш */,
    "щ" /* 0xF9: щ */,
    "ъ" /* 0xFA: ъ */,
    "ы" /* 0xFB: ы */,
    "ь" /* 0xFC: ь */,
    "э" /* 0xFD: э */,
    "ю" /* 0xFE: ю */,
    "я" /* 0xFF: я */
};

void escapeString(MemoryWriter& dest, const char* begin, const char* end)
{
    while(begin != end){
        const char* str = escapeTable[(unsigned char)(*begin)];
        dest.write(str);
        ++begin;
    }
}

// ---------------------------------------------------------------------------

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
	*buffer_ << "{\r\n";
}

void TextOArchive::closeBracket()
{
	*buffer_ << "}\r\n";
}

void TextOArchive::openContainerBracket()
{
    *buffer_ << "[\r\n";
}

void TextOArchive::closeContainerBracket()
{
    *buffer_ << "]\r\n";
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
    *buffer_ << "\r\n";
    return true;
}


bool TextOArchive::operator()(std::string& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << "\""; 
    const char* str = value.c_str();
    escapeString(*buffer_, str, str + value.size());
    (*buffer_) << "\"\r\n";
    return true;
}

bool TextOArchive::operator()(float& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(double& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(int& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(unsigned int& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(short& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(unsigned short& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(__int64& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(unsigned char& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(signed char& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(char& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << value << "\r\n";
    return true;
}

bool TextOArchive::operator()(StringListStaticValue& value, const char* name, const char* label)
{
    placeIndent();
    placeName(name);
    (*buffer_) << "\"" << value.c_str() << "\"\r\n";
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
        if(*next != '\t' && *next != '\r'){
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
