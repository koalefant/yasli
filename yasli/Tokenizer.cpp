#include "StdAfx.h"
#include "yasli/Tokenizer.h"
#include <iostream>

// #define DEBUG_TOKENIZER

namespace yasli{

Tokenizer::Tokenizer()
{
}

inline bool Tokenizer::isSpace(char c) const
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool Tokenizer::isComment(char c) const
{
	return c == '#';
}


inline bool Tokenizer::isQuote(char c) const
{
	return c == '\"';
}

static const char charTypes[256] = {
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
	0 /* 0x2D: - */,
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


	0 /* 0xD0: Р */,
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

inline bool Tokenizer::isWordPart(unsigned char c) const
{
    return charTypes[c] != 0;
}

Token Tokenizer::operator()(const char* ptr) const
{
	while(isSpace(*ptr))
		++ptr;
	Token cur(ptr, ptr);
	while(!cur && *ptr != '\0'){
		while(isComment(*cur.end)){
#ifdef DEBUG_TOKENIZER
			const char* commentStart = ptr;
#endif
			while(*cur.end && *cur.end != '\n')
				++cur.end;
			while(isSpace(*cur.end))
				++cur.end;
#ifdef DEBUG_TOKENIZER
			std::cout << "Got comment: '" << std::string(commentStart, cur.end) << "'" << std::endl;
#endif
			cur.start = cur.end;
		}
		ASSERT(!isSpace(*cur.end));
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
				}
				if(isQuote(*cur.end)){
					++cur.end;
#ifdef DEBUG_TOKENIZER
					std::cout << "Tokenizer result: " << cur.str() << std::endl;
#endif
					return cur;
				}
				else
					++cur.end;
			}
		}
		else{
			//ASSERT(*cur.end);
			if(!*cur.end)
				return cur;

#ifdef DEBUG_TOKENIZER
			char twoChars[] = { *cur.end, '\0' };
			std::cout << twoChars << std::endl;
#endif
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
