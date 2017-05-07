/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <limits>
#include <math.h>
#include <float.h>
#include "yasli/STL.h"
#include "yasli/BlackBox.h"
#include "yasli/Config.h"
#include "JSONIArchive.h"
#include "MemoryReader.h"

#ifdef _MSC_VER
#ifndef NAN
	static unsigned long g_nan[2] = {0xffffffff, 0x7fffffff};
	#define NAN (*(double*)g_nan)
#endif
#endif

#if 0
# define DEBUG_TRACE(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
# define DEBUG_TRACE_TOKENIZER(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
# define DEBUG_TRACE(...)
# define DEBUG_TRACE_TOKENIZER(...)
#endif

namespace yasli{

inline bool isDigit(int ch) 
{
	return unsigned(ch - '0') <= 9;
}

double parseFloat(const char* s)
{
	double res = 0, f = 1, sign = 1;
	while(*s && (*s == ' ' || *s == '\t')) 
		s++;

	if(*s == '-') 
		sign=-1, s++; 
	else if (*s == '+') 
		s++;

	for(; isDigit(*s); s++)
		res = res * 10 + (*s - '0');

	if(*s == '.')
		for (s++; isDigit(*s); s++)
			res += (f *= 0.1)*(*s - '0');
	return res*sign;
}

namespace json_local {

static char hexValueTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,

    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

}

static void unescapeString(std::vector<char>& buf, string& out, const char* begin, const char* end)
{
	if(begin >= end) {
		out.clear();
		return;
	}
	// TODO: use stack string
	buf.resize(end-begin);
	char* ptr = buf.empty() ? 0 : &buf.front();
	while(begin != end){
		if(*begin != '\\'){
			*ptr = *begin;
			++ptr;
		}
		else{
			++begin;
			if(begin == end)
				break;

			switch(*begin){
			case '0':  *ptr = '\0'; ++ptr; break;
			case 't':  *ptr = '\t'; ++ptr; break;
			case 'n':  *ptr = '\n'; ++ptr; break;
			case 'r':  *ptr = '\r'; ++ptr; break;
			case '\\': *ptr = '\\'; ++ptr; break;
			case '\"': *ptr = '\"'; ++ptr; break;
			case '\'': *ptr = '\''; ++ptr; break;
			case 'x':
								 if(begin + 2 < end){
									 *ptr = (json_local::hexValueTable[int(begin[1])] << 4) + json_local::hexValueTable[int(begin[2])];
									 ++ptr;
									 begin += 2;
									 break;
								 }
			default:
								 *ptr = *begin;
								 ++ptr;
								 break;
			}
		}
		++begin;
	}
	buf.resize(ptr - (buf.empty() ? 0 : &buf.front()));
	if (!buf.empty())
		out.assign(&buf[0], &buf[0] + buf.size());
	else
		out.clear();
}

static Token get_line_content(const char* buffer, int line) {
	const char* start = buffer;
	for (int i = 0; i < line - 1; ++i) {
		const char* p = start;
		while (true) {
			if (*p == '\0')
				return Token();
			if (*p == '\n') {
				++p;
				break;
			}
			++p;
		}
		start = p;
	}
	const char* p = start;
	while (true) {
		if (*p == '\0' || *p == '\n')
			break;
		++p;
	}
	return Token(start, p);
}

static void print_line_with_underlined_token(const char* buffer, int line_no, const Token& token) {
	Token line_content = get_line_content(buffer, line_no);
	fprintf(stderr, "\t%s\n", line_content.str().c_str());
	string line_underline;
	line_underline.reserve(line_content.length());
	for (const char* p = line_content.start; p != line_content.end; ++p) {
		if (p == token.start)
			line_underline.push_back('^');
		else if (p > token.start && p < token.end)
			line_underline.push_back('~');
		else if (*p == '\t' || *p == '\n' || *p == '\r')
			line_underline.push_back(*p);
		else
			line_underline.push_back(' ');
	}
	fprintf(stderr, "\t%s\n", line_underline.c_str());
}

// ---------------------------------------------------------------------------

class JSONTokenizer{
public:
    JSONTokenizer();

    Token operator()(const char* text) const;
private:
    inline static bool isSpace(char c);
    inline static bool isWordPart(unsigned char c);
    inline static bool isComment(char c);
    inline static bool isQuoteOpen(int& quoteIndex, char c);
    inline static bool isQuoteClose(int quoteIndex, char c);
    inline static bool isQuote(char c);
};

JSONTokenizer::JSONTokenizer()
{
}

inline bool JSONTokenizer::isSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool JSONTokenizer::isComment(char c)
{
	return c == '#';
}

inline bool JSONTokenizer::isQuote(char c)
{
	return c == '\"';
}

static const char jsonCharTypes[256] = {
	0 /* 0x00: */,
	0 /* 0x01: */,
	0 /* 0x02: */,
	0 /* 0x03: */,
	0 /* 0x04: */,
	0 /* 0x05: */,
	0 /* 0x06: */,
	0 /* 0x07: */,
	0 /* 0x08: */,
	0 /* 0x09: \t */,
	0 /* 0x0A: \n */,
	0 /* 0x0B: */,
	0 /* 0x0C: */,
	0 /* 0x0D: */,
	0 /* 0x0E: */,
	0 /* 0x0F: */,


	0 /* 0x10: */,
	0 /* 0x11: */,
	0 /* 0x12: */,
	0 /* 0x13: */,
	0 /* 0x14: */,
	0 /* 0x15: */,
	0 /* 0x16: */,
	0 /* 0x17: */,
	0 /* 0x18: */,
	0 /* 0x19: */,
	0 /* 0x1A: */,
	0 /* 0x1B: */,
	0 /* 0x1C: */,
	0 /* 0x1D: */,
	0 /* 0x1E: */,
	0 /* 0x1F: */,


	0 /* 0x20:   */,
	0 /* 0x21: ! */,
	0 /* 0x22: " */,
	0 /* 0x23: # */,
	0 /* 0x24: $ */,
	0 /* 0x25: % */,
	0 /* 0x26: & */,
	0 /* 0x27: ' */,
	0 /* 0x28: ( */,
	0 /* 0x29: ) */,
	0 /* 0x2A: * */,
	0 /* 0x2B: + */,
	0 /* 0x2C: , */,
	1 /* 0x2D: - */,
	1 /* 0x2E: . */,
	0 /* 0x2F: / */,


	1 /* 0x30: 0 */,
	1 /* 0x31: 1 */,
	1 /* 0x32: 2 */,
	1 /* 0x33: 3 */,
	1 /* 0x34: 4 */,
	1 /* 0x35: 5 */,
	1 /* 0x36: 6 */,
	1 /* 0x37: 7 */,
	1 /* 0x38: 8 */,
	1 /* 0x39: 9 */,
	0 /* 0x3A: : */,
	0 /* 0x3B: ; */,
	0 /* 0x3C: < */,
	0 /* 0x3D: = */,
	0 /* 0x3E: > */,
	0 /* 0x3F: ? */,


	0 /* 0x40: @ */,
	1 /* 0x41: A */,
	1 /* 0x42: B */,
	1 /* 0x43: C */,
	1 /* 0x44: D */,
	1 /* 0x45: E */,
	1 /* 0x46: F */,
	1 /* 0x47: G */,
	1 /* 0x48: H */,
	1 /* 0x49: I */,
	1 /* 0x4A: J */,
	1 /* 0x4B: K */,
	1 /* 0x4C: L */,
	1 /* 0x4D: M */,
	1 /* 0x4E: N */,
	1 /* 0x4F: O */,


	1 /* 0x50: P */,
	1 /* 0x51: Q */,
	1 /* 0x52: R */,
	1 /* 0x53: S */,
	1 /* 0x54: T */,
	1 /* 0x55: U */,
	1 /* 0x56: V */,
	1 /* 0x57: W */,
	1 /* 0x58: X */,
	1 /* 0x59: Y */,
	1 /* 0x5A: Z */,
	0 /* 0x5B: [ */,
	0 /* 0x5C: \ */,
	0 /* 0x5D: ] */,
	0 /* 0x5E: ^ */,
	1 /* 0x5F: _ */,


	0 /* 0x60: ` */,
	1 /* 0x61: a */,
	1 /* 0x62: b */,
	1 /* 0x63: c */,
	1 /* 0x64: d */,
	1 /* 0x65: e */,
	1 /* 0x66: f */,
	1 /* 0x67: g */,
	1 /* 0x68: h */,
	1 /* 0x69: i */,
	1 /* 0x6A: j */,
	1 /* 0x6B: k */,
	1 /* 0x6C: l */,
	1 /* 0x6D: m */,
	1 /* 0x6E: n */,
	1 /* 0x6F: o */,


	1 /* 0x70: p */,
	1 /* 0x71: q */,
	1 /* 0x72: r */,
	1 /* 0x73: s */,
	1 /* 0x74: t */,
	1 /* 0x75: u */,
	1 /* 0x76: v */,
	1 /* 0x77: w */,
	1 /* 0x78: x */,
	1 /* 0x79: y */,
	1 /* 0x7A: z */,
	0 /* 0x7B: { */,
	0 /* 0x7C: | */,
	0 /* 0x7D: } */,
	0 /* 0x7E: ~ */,
	0 /* 0x7F: */,


	0 /* 0x80: */,
	0 /* 0x81: */,
	0 /* 0x82: */,
	0 /* 0x83: */,
	0 /* 0x84: */,
	0 /* 0x85: */,
	0 /* 0x86: */,
	0 /* 0x87: */,
	0 /* 0x88: */,
	0 /* 0x89: */,
	0 /* 0x8A: */,
	0 /* 0x8B: */,
	0 /* 0x8C: */,
	0 /* 0x8D: */,
	0 /* 0x8E: */,
	0 /* 0x8F: */,


	0 /* 0x90: */,
	0 /* 0x91: */,
	0 /* 0x92: */,
	0 /* 0x93: */,
	0 /* 0x94: */,
	0 /* 0x95: */,
	0 /* 0x96: */,
	0 /* 0x97: */,
	0 /* 0x98: */,
	0 /* 0x99: */,
	0 /* 0x9A: */,
	0 /* 0x9B: */,
	0 /* 0x9C: */,
	0 /* 0x9D: */,
	0 /* 0x9E: */,
	0 /* 0x9F: */,


	0 /* 0xA0: */,
	0 /* 0xA1: Ў */,
	0 /* 0xA2: ў */,
	0 /* 0xA3: Ј */,
	0 /* 0xA4: ¤ */,
	0 /* 0xA5: Ґ */,
	0 /* 0xA6: ¦ */,
	0 /* 0xA7: § */,
	0 /* 0xA8: Ё */,
	0 /* 0xA9: © */,
	0 /* 0xAA: Є */,
	0 /* 0xAB: « */,
	0 /* 0xAC: ¬ */,
	0 /* 0xAD: ­ */,
	0 /* 0xAE: ® */,
	0 /* 0xAF: Ї */,


	0 /* 0xB0: ° */,
	0 /* 0xB1: ± */,
	0 /* 0xB2: І */,
	0 /* 0xB3: і */,
	0 /* 0xB4: ґ */,
	0 /* 0xB5: µ */,
	0 /* 0xB6: ¶ */,
	0 /* 0xB7: · */,
	0 /* 0xB8: ё */,
	0 /* 0xB9: № */,
	0 /* 0xBA: є */,
	0 /* 0xBB: » */,
	0 /* 0xBC: ј */,
	0 /* 0xBD: Ѕ */,
	0 /* 0xBE: ѕ */,
	0 /* 0xBF: ї */,


	0 /* 0xC0: А */,
	0 /* 0xC1: Б */,
	0 /* 0xC2: В */,
	0 /* 0xC3: Г */,
	0 /* 0xC4: Д */,
	0 /* 0xC5: Е */,
	0 /* 0xC6: Ж */,
	0 /* 0xC7: З */,
	0 /* 0xC8: И */,
	0 /* 0xC9: Й */,
	0 /* 0xCA: К */,
	0 /* 0xCB: Л */,
	0 /* 0xCC: М */,
	0 /* 0xCD: Н */,
	0 /* 0xCE: О */,
	0 /* 0xCF: П */,


	0 /* 0xD0: �  */,
	0 /* 0xD1: С */,
	0 /* 0xD2: Т */,
	0 /* 0xD3: У */,
	0 /* 0xD4: Ф */,
	0 /* 0xD5: Х */,
	0 /* 0xD6: Ц */,
	0 /* 0xD7: Ч */,
	0 /* 0xD8: Ш */,
	0 /* 0xD9: Щ */,
	0 /* 0xDA: Ъ */,
	0 /* 0xDB: Ы */,
	0 /* 0xDC: Ь */,
	0 /* 0xDD: Э */,
	0 /* 0xDE: Ю */,
	0 /* 0xDF: Я */,


	0 /* 0xE0: а */,
	0 /* 0xE1: б */,
	0 /* 0xE2: в */,
	0 /* 0xE3: г */,
	0 /* 0xE4: д */,
	0 /* 0xE5: е */,
	0 /* 0xE6: ж */,
	0 /* 0xE7: з */,
	0 /* 0xE8: и */,
	0 /* 0xE9: й */,
	0 /* 0xEA: к */,
	0 /* 0xEB: л */,
	0 /* 0xEC: м */,
	0 /* 0xED: н */,
	0 /* 0xEE: о */,
	0 /* 0xEF: п */,


	0 /* 0xF0: р */,
	0 /* 0xF1: с */,
	0 /* 0xF2: т */,
	0 /* 0xF3: у */,
	0 /* 0xF4: ф */,
	0 /* 0xF5: х */,
	0 /* 0xF6: ц */,
	0 /* 0xF7: ч */,
	0 /* 0xF8: ш */,
	0 /* 0xF9: щ */,
	0 /* 0xFA: ъ */,
	0 /* 0xFB: ы */,
	0 /* 0xFC: ь */,
	0 /* 0xFD: э */,
	0 /* 0xFE: ю */,
	0 /* 0xFF: я */
};

inline bool JSONTokenizer::isWordPart(unsigned char c)
{
		return jsonCharTypes[c] != 0;
}

Token JSONTokenizer::operator()(const char* ptr) const
{
	while(isSpace(*ptr))
		++ptr;
	Token cur(ptr, ptr);
	while(!cur && *ptr != '\0'){
		while(isComment(*cur.end)){
			const char* commentStart = ptr;
			while(*cur.end && *cur.end != '\n')
				++cur.end;
			while(isSpace(*cur.end))
				++cur.end;
			DEBUG_TRACE_TOKENIZER("Got comment: '%s'", string(commentStart, cur.end).c_str());
			cur.start = cur.end;
		}
		YASLI_ASSERT(!isSpace(*cur.end));
		if(isQuote(*cur.end)){
			++cur.end;
			while(*cur.end){ 
				if(*cur.end == '\\'){
					++cur.end;
					if(*cur.end ){
						if(*cur.end != 'x' && *cur.end != 'X')
							++cur.end;
						else{
							++cur.end;
							if(*cur.end)
								++cur.end;
						}
					}
                    continue;
				}
				if(isQuote(*cur.end)){
					++cur.end;
					DEBUG_TRACE_TOKENIZER("Tokenizer result: '%s'", cur.str().c_str());
					return cur;
				}
				else
					++cur.end;
			}
		}
		else{
			//YASLI_ASSERT(*cur.end);
			if(!*cur.end)
				return cur;

			DEBUG_TRACE_TOKENIZER("%d", (int)*cur.end);
			if(isWordPart(*cur.end))
			{
				do{
					++cur.end;
				} while(isWordPart(*cur.end) != 0);
			}
			else
			{
				++cur.end;
				return cur;
			}
			DEBUG_TRACE_TOKENIZER("Tokenizer result: '%s'", cur.str().c_str());
			return cur;
		}
	}
	DEBUG_TRACE_TOKENIZER("Tokenizer result: '%s'", cur.str().c_str());
	return cur;
}


// ---------------------------------------------------------------------------

JSONIArchive::JSONIArchive()
: Archive(INPUT | TEXT | VALIDATION)
{
}

JSONIArchive::~JSONIArchive()
{
	if (buffer_) {
		free(buffer_);
		buffer_ = 0;
	}
	reader_.reset();
}

bool JSONIArchive::open(const char* buffer, size_t length, bool free)
{
	if(!length)
		return false;

	if(buffer)
		reader_.reset(new MemoryReader(buffer, length, free));
	buffer_ = 0;

	token_ = Token(reader_->begin(), reader_->begin());

	stack_ = &stackBottom_;
	readToken();
	putToken();
	stack_->start = token_.end;
	unusedFieldCount_ = 0;
	return true;
}


bool JSONIArchive::load(const char* filename)
{
	if(FILE* file = fopen(filename, "rb"))
	{
		fseek(file, 0, SEEK_END);
		long fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		void* buffer = 0;
		if(fileSize > 0){
			buffer = malloc(fileSize + 1);
			YASLI_ASSERT(buffer != 0);
			memset(buffer, 0, fileSize + 1);
			size_t elementsRead = fread(buffer, fileSize, 1, file);
			YASLI_ASSERT(((char*)(buffer))[fileSize] == '\0');
			if(elementsRead != 1){
				free(buffer);
				fclose(file);
				return false;
			}
		}
		fclose(file);

		filename_ = filename;
		buffer_ = buffer;
		if (fileSize > 0)
			return open((char*)buffer, fileSize, true);
		else
			return false; 
	}
	else{
		return false;
	}
}

void JSONIArchive::readToken()
{
	JSONTokenizer tokenizer;
	token_ = tokenizer(token_.end);
}

void JSONIArchive::putToken()
{
    token_ = Token(token_.start, token_.start);
}

int JSONIArchive::line(int* column_no, const char* position) const
{
	int line_no = 1;
	const char* last_line = position;
	for (const char* p = reader_->begin(); p != position; ++p) {
		if (*p == '\n') {
			++line_no;
			last_line = p;
		}
	}
	if (column_no) {
		*column_no = int(position - last_line);
	}
	return line_no;
}

bool JSONIArchive::isName(Token token) const
{
	if(!token)
		return false;
	char firstChar = token.start[0];
	if (firstChar == '"')
		return true;
	return false;
}


bool JSONIArchive::expect(char token)
{
	if (token_ == token){
		return true;
	}

	const char* lineEnd = token_.start;
	while (lineEnd && *lineEnd != '\0' && *lineEnd != '\r' && *lineEnd != '\n')
		++lineEnd;
	error(0, TypeID(), "Error parsing file, expected ':' at line %d, got '%s'\n",
		  line(nullptr, token_.start), string(token_.start, lineEnd).c_str());
	return false;
}

void JSONIArchive::skipBlock()
{
	DEBUG_TRACE("Skipping block from %i ...", int(token_.end - reader_->begin()));
    if(openBracket() || openContainerBracket())
        closeBracket(); // Skipping entire block
    else
        readToken(); // Skipping value
	readToken();
	if (token_ != ',')
		putToken();
	DEBUG_TRACE(" ...till %i", int(token_.end - reader_->begin()));
	DEBUG_TRACE("Skipped");
}

bool JSONIArchive::findName(const char* name, Token* outName, bool untilEndOfBlockOnly)
{
	DEBUG_TRACE(" * finding name '%s'", name);
	DEBUG_TRACE("   started at byte %i", int(token_.start - reader_->begin()));
	if(stack_ == nullptr) {
		// TODO: diagnose
		return false;
	}
	Level& level = *stack_;
	if (level.isKeyValue)
		return true;
	const char* blockBegin = level.start;
	if(*blockBegin == '\0')
		return false;

	bool bottomOfStack = stack_ == &stackBottom_;
	if(bottomOfStack || level.isContainer || outName != 0){
		// root or container or next value in a key-pair
		readToken();
		if (token_ == ',')
			readToken();
		// root level or inside of the container, or a structure that pretends to be an array by
		// specifying empty names
		if(token_ == ']' || token_ == '}'){
			DEBUG_TRACE("Got close bracket...");
			putToken();
			return false;
		}
		else{
			DEBUG_TRACE("Got unnamed value: '%s'", token_.str().c_str());
			putToken();
			return true;
		}
	}

	if (!untilEndOfBlockOnly && !bottomOfStack) {
		if (name[0] == '\0' && !level.isContainer) {
			readToken();
			error(0, TypeID(), "Deserializing nameless field in dictionary block");
			putToken();
		}
		if (name[0] != '\0' && level.isContainer) {
			readToken();
			error(0, TypeID(), "Deserializing named field \"%s\" in array block", name);
			putToken();
		}
	}

	Token start;
	while(true){
		readToken();
		if (token_ == ',')
			readToken();
		if(!token_){
			// end of file
			error(0, TypeID(), "Unexpected end of file");
			return false;
		}
		DEBUG_TRACE("token '%s'", token_.str().c_str());
		DEBUG_TRACE("Checking for loop: %d and %d, start %s", int(token_.start - reader_->begin()), int(start.start - reader_->begin()), start.str().c_str());

		if(token_ == start){
			// we have seen this token before, it is a full circle and we haven't found the name we
			// were looking for
			putToken();
			DEBUG_TRACE("unable to find...");
			return false;
		} else {
			// initialize start
			if (start == Token()) {
				DEBUG_TRACE("initialized start to '%s'", token_.str().c_str());
				start = token_;
			}
		}

		if(token_ == '}' || token_ == ']'){ // CONVERSION
			DEBUG_TRACE("Going to begin of block, from %d", int(token_.start - reader_->begin()));
			YASLI_ASSERT(start != Token());
			token_ = Token(blockBegin, blockBegin);
			level.fieldIndex = 0;
			level.parsedBlock = true;
			DEBUG_TRACE(" to %i", int(token_.start - reader_->begin()));
			if (untilEndOfBlockOnly) {
				return false;
			}
			continue; // Reached '}' or ']' while searching for name, continue from begin of block
		}

		if(name[0] == '\0' && !untilEndOfBlockOnly){
			if(isName(token_)){
				readToken();
				if(!token_)
					return false; // Reached end of file while searching for name
				expect(':');
				skipBlock();
			}
			else{
				putToken(); // Not a name - that is what we are looking for
				return true;
			}
		}
		else{
			if(isName(token_)){
				Token nameToken = token_;
				Token nameContent(token_.start+1, token_.end-1);
				readToken();
				expect(':');
				if(nameContent == name) {
					if (outName)
						*outName = nameContent;
					if (warnAboutUnusedFields_) {
						// mark name as used
						if (level.fieldIndex < int(level.names.size())) {
							level.names[level.fieldIndex] = Token();
						} else {
							YASLI_ASSERT(level.names.size() == level.fieldIndex);
							level.names.push_back(Token());
						}
						++level.fieldIndex;
					}
					return true;
				}
				else {
					skipBlock();
					if (warnAboutUnusedFields_) {
						// mark name as unused
						if (level.fieldIndex >= int(level.names.size())) {
							YASLI_ASSERT(level.names.size() == level.fieldIndex);
							level.names.push_back(nameToken);
						}
						++level.fieldIndex;
					}
				}

			}
			else{
				putToken();
				skipBlock();
			}
		}

	}

	return false;
}

bool JSONIArchive::openBracket()
{
	readToken();
	if (token_ == '{')
		return true;
	putToken();
    return false;
}

bool JSONIArchive::closeBracket()
{
    int relativeLevel = 0;
    while(true){
        readToken();
		if (token_ == ',')
			readToken();
        if(!token_){
            const char* start = stack_->start;
			int column = 0;
			error(nullptr, TypeID(), "No closing bracket found for line %d\n", line(nullptr, start));
			return false;
        }
		else if(token_ == '}' || token_ == ']'){ // CONVERSION
            if(relativeLevel == 0)
				return true;
            else
                --relativeLevel;
        }
		else if(token_ == '{' || token_ == '['){ // CONVERSION
            ++relativeLevel;
        }
    }
    return false;
}

bool JSONIArchive::openContainerBracket()
{
	readToken();
	if(token_ == '[')
		return true;
	putToken();
	return false;
}

bool JSONIArchive::closeContainerBracket()
{
    readToken();
    if(token_ == ']'){
		DEBUG_TRACE("closeContainerBracket(): ok");
        return true;
    }
    else{
		DEBUG_TRACE("closeContainerBracket(): failed ('%s')", token_.str().c_str());
        putToken();
        return false;
    }
}

bool JSONIArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
    if(findName(name)){
		Level level(stack_);
		readToken();
		bool isContainer = token_ == '[';
		if (!isContainer && token_ != '{') {
			putToken();
			return false;
		}
		level.start = token_.end;
		level.isContainer = isContainer;

        if (ser)
            ser(*this);

		if (warnAboutUnusedFields_) {
			Level& level = *stack_;

			// read remaining names
			if (!level.parsedBlock) {
				findName("", nullptr, true);
			}

			int numFields = level.names.size();
			for (int i = 0; i < numFields; ++i) {
				Token token = level.names[i];
				if (token != Token()) {
					int column_no = 0;
					int line_no = line(&column_no, token.start);
					fprintf(stderr, "%s:%d:%d: error: unused field %s while deserializing type %s\n",
							filename_.c_str(), line_no, column_no, token.str().c_str(), ser.type().name());
					print_line_with_underlined_token(reader_->buffer(), line_no, token);
					++unusedFieldCount_;
				}
			}
		}
        closeBracket();
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(const BlackBox& box, const char* name, const char* label)
{
	if(findName(name)){
		if(openBracket() || openContainerBracket()){
			const char* start = token_.start;
			putToken();
			skipBlock();
			const char* end = token_.start;
			if (end < start) {
				YASLI_ASSERT(0);
				return false;
			}
			while (end > start && 
				(*(end - 1) == ' '
				|| *(end - 1) == '\r'
				|| *(end - 1) == '\n'
				|| *(end - 1) == '\t'))
				--end;
			// box has to be const in the interface so we can serialize
			// temporary variables (i.e. function call result or structures
			// constructed on the stack)
			const_cast<BlackBox&>(box).set("json", (void*)start, end - start);
			return true;
		}
	}
    return false;
}

bool JSONIArchive::operator()(PointerInterface& ser, const char* name, const char* label)
{
	if (findName(name)) {
		if(openBracket()){
			Level level(stack_);
			level.start = token_.end;
			level.isKeyValue = true;

			readToken();
			if (isName(token_)) {
				if(checkStringValueToken()){
					string typeName;
					unescapeString(unescapeBuffer_, typeName, token_.start + 1, token_.end - 1);

					if (typeName != ser.registeredTypeName())
						ser.create(typeName.c_str());
					readToken();
					expect(':');
					operator()(ser.serializer(), "", 0);
				}
			}
			else {
				putToken();

				ser.create("");				
			}
			closeBracket();
			return true;
		}
	}
	return false;
}


bool JSONIArchive::operator()(ContainerInterface& ser, const char* name, const char* label)
{
    if(findName(name)){
		Token bracketToken = token_;
		bool containerBracket = openContainerBracket();
		bool dictionaryBracket = false;
		if (!containerBracket)
			dictionaryBracket = openBracket();
		if(containerBracket || dictionaryBracket){
			Level level(stack_);
			if (containerBracket) {
				level.isContainer = true;
			} else if (dictionaryBracket) {
				level.isDictionary = true;
			}
			level.start = token_.end;

			std::size_t size = ser.size();
			std::size_t index = 0;

			while(true){
				readToken();
				if (token_ == ',')
					readToken();
				if (token_ == '}') {
					if (!dictionaryBracket) {
						error(nullptr, TypeID(), "Unexpected bracket");
					}
					break;
				}
				if(token_ == ']') {
					if (!containerBracket) {
						error(nullptr, TypeID(), "Unexpected brace");
					}
					break;
				}
				else if(!token_)
				{
					error(nullptr, TypeID(), "Reached end of file while reading container");
					return false;
				}
				putToken();
				if(index == size)
					size = index + 1;
				if(index < size){
					if (!ser(*this, "", "")) {
						// We've got a named item within a container, 
						// i.e. looks like a dictionary but not a container.
						// Bail out, it is nothing we can do here.
						closeBracket();
						break;
					}
				}
				else{
					skipBlock();
				}
				ser.next();
				++index;
			}
			if(size > index)
				ser.resize(index);
			return true;
		} else {
			warning(nullptr, TypeID(), "Non-container value where container is expected");
		}
    }
    return false;
}

bool JSONIArchive::operator()(MapInterface& ser, const char* name, const char* label)
{
    if (findName(name)) {
		Token bracketToken = token_;

		if (ser.keySerializesToString()) {
			// expect { KEY: VALUE, ... }
			if (!openBracket()) {
				error(nullptr, TypeID(), "Expecting opening brace '{'");
				return false;
			}

			Level level(stack_);
			level.isKeyValue = true;
			level.start = token_.end;

			bool missingComma = false;
			bool first = true;
			while(true){
				readToken();
				if (!first) {
					if (token_ == ',') {
						readToken();
					} else {
						missingComma = true;
					}
				}
				if (token_ == '}')
					break;
				putToken();
				ser.deserializeNewKey(*this, "", "");
				readToken();
				if (!expect(':'))
					break;
				ser.deserializeNewValue(*this, "", "");
				ser.next();
				first = false;
			}

		} else {
			// expect [ { "key": KEY, "value": VALUE }, ... ]
			if (!openContainerBracket()) {
				warning(nullptr, TypeID(), "Expecting an opening square bracket");
			}
			Level outer_level(stack_);
			outer_level.isContainer = true;
			outer_level.start = token_.end;

			Level level(stack_);
			level.isContainer = false;
			level.isKeyValue = true;
			level.start = token_.end;

			bool first = true;
			bool missingComma = false;
			while(true){
				readToken();
				if (!first) {
					if (token_ == ',') {
						readToken();
					} else {
						missingComma = true;
					}
				}
				if (token_ == '{') {
					if (missingComma) {
						error(nullptr, TypeID(), "Missing comma before block");
					}
					if (!ser.deserializeNewKey(*this, "key", "")) {
						error(nullptr, TypeID(), "Missing expected key");
						break;
					}
					readToken();
					if (token_ != ',') {
						error(nullptr, TypeID(), "Expected comma");
						putToken();
					}
					if (!ser.deserializeNewValue(*this, "value", "") ){
						error(nullptr, TypeID(), "Missing expected value");
						break;
					}
					ser.next();
					readToken();
					if (!expect('}'))
						break;
				} else if (token_ == ']') {
					break;
				} else if(!token_) {
					error(nullptr, TypeID(), "Reached end of file while reading container");
					return false;
				} else {
					error(nullptr, TypeID(), "Unexpected token, expecting '{'");
				}
				first = false;
			}
			return true;
		}
	}
    return false;
}

void JSONIArchive::checkValueToken()
{
    if(!token_){
		error(nullptr, TypeID(), "End of while while expecting value token");
    }
}

void JSONIArchive::checkIntegerToken()
{
    if(!token_){
		error(nullptr, TypeID(), "Reached end of file while expecting a value token");
    } else {
		const char* p = token_.start;
		for (const char* p = token_.start; p != token_.end; ++p) {
			if (*p != '-' && *p != '+' && !(*p >= '0' && *p <= '9')) {
				error(nullptr, TypeID(), "Expecting an integer instead of %s", string(token_.start, token_.end).c_str());
				return;
			}
		}
	}
}

bool JSONIArchive::checkStringValueToken()
{
    if(!token_){
		error(nullptr, TypeID(), "End of file while expecting string");
		return false;
    }
    if(token_.start[0] != '"' || token_.end[-1] != '"'){
		error(nullptr, TypeID(), "Expecting string instead of %s", token_.str().c_str());
		return false;
    }
    return true;
}

bool JSONIArchive::operator()(i32& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
        value = strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(u32& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
        value = strtoul(token_.start, 0, 10);
        return true;
    }
    return false;
}


bool JSONIArchive::operator()(i16& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
        value = (short)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(u16& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
        value = (unsigned short)strtoul(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(i64& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
#ifdef _MSC_VER
		value = _strtoi64(token_.start, 0, 10);
#else
		value = strtoll(token_.start, 0, 10);
#endif
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(u64& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
#ifdef _MSC_VER
		value = _strtoui64(token_.start, 0, 10);
#else
		value = strtoull(token_.start, 0, 10);
#endif
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(float& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
		if (*token_.start != '\"') {
#ifdef _MSC_VER
			value = (float)std::atof(token_.start);
#else
			value = strtof(token_.start, 0);
#endif
		}
		else if (strncmp(token_.start, "\"Infinity\"", 10) == 0)
			value = std::numeric_limits<float>::infinity();
		else if (strncmp(token_.start, "\"-Infinity\"", 11) == 0)
			value = -std::numeric_limits<float>::infinity();
		else if (strncmp(token_.start, "\"NaN\"", 5) == 0)
			value = (float)NAN;
		else
			return false;
		return true;
    }
    return false;
}

bool JSONIArchive::operator()(double& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
		if (*token_.start != '\"') {
#ifdef _MSC_VER
			value = std::atof(token_.start);
#else
			value = strtod(token_.start, 0);
#endif
		}
		else if (strncmp(token_.start, "\"Infinity\"", 10) == 0)
			value = std::numeric_limits<double>::infinity();
		else if (strncmp(token_.start, "\"-Infinity\"", 11) == 0)
			value = -std::numeric_limits<double>::infinity();
		else if (strncmp(token_.start, "\"NaN\"", 5) == 0)
			value = NAN;
		else
			return false;
		return true;
    }
    return false;
}

bool JSONIArchive::operator()(StringInterface& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        if(checkStringValueToken()){
			string buf;
			unescapeString(unescapeBuffer_, buf, token_.start + 1, token_.end - 1);
			value.set(buf.c_str());
		}
		else
			return false;
        return true;
    }
    return false;
}


namespace json_local {

inline size_t utf8InUtf16Len(const char* p)
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

inline const char* readUtf16FromUtf8(unsigned int* ch, const char* s)
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


inline void utf8ToUtf16(wstring* out, const char* in)
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

}

bool JSONIArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
	if(findName(name)){
		readToken();
		if(checkStringValueToken()){
			string buf;
			unescapeString(unescapeBuffer_, buf, token_.start + 1, token_.end - 1);
			wstring wbuf;
			json_local::utf8ToUtf16(&wbuf, buf.c_str());
			value.set(wbuf.c_str());
		}
		else
			return false;
		return true;
	}
	return false;

}

bool JSONIArchive::operator()(bool& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkValueToken();
        if(token_ == "true")
            value = true;
        else if(token_ == "false")
            value = false;
		else {
			error(nullptr, TypeID(), "Expecting true or false");
		}
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(i8& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
        value = (i8)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(u8& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
        value = (u8)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

bool JSONIArchive::operator()(char& value, const char* name, const char* label)
{
    if(findName(name)){
        readToken();
        checkIntegerToken();
        value = (char)strtol(token_.start, 0, 10);
        return true;
    }
    return false;
}

void JSONIArchive::setDebugFilename(const char* filename) {
	filename_ = filename;
}

void JSONIArchive::validatorMessage(bool error, const void* handle, const TypeID& type, const char* message) {
	if (disableWarnings_)
		return;
	if (token_.start) {
		int column_no = 0;
		int line_no = line(&column_no, token_.start);
		fprintf(stderr, "%s:%d:%d: %s: %s\n", filename_.c_str(), line_no, column_no, error?"error":"warning", message);
		print_line_with_underlined_token(reader_->buffer(), line_no, token_);
	} else {
		fprintf(stderr, "%s: %s: %s\n", filename_.c_str(), error?"error":"warning", message);
	}
}

}
// vim:ts=4 sw=4:
