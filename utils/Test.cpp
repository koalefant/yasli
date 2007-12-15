#include "StdAfx.h"
#include "testo/Testo.h"
#include <string>
#include <iostream>
#include <math.h>
#include "utils/Tokenizer.h"
#include "utils/StringUtil.h"

#include "utils/SharedLibApi.h"

#ifndef M_PI
# define M_PI 3.1415926
#endif

void EXPORT_SYMBOL dummyExport()
{
	return;
}



TESTO_BEGIN()

struct TokenizerQuotes{
    void invoke(){
        const char* text = "first \"doubleQuoted\'\"\t\'single\"\"Quoted\' \"not closed quotes\'";

        Tokenizer tokenizer(" \t", "\"\"\'\'", "");

        Token token;
        token = tokenizer(text);
        TESTO_ENSURE(token == "first");
        token = tokenizer(token.end());
        TESTO_ENSURE(token == "\"doubleQuoted\'\"");
        token = tokenizer(token.end());
        TESTO_ENSURE(token == "\'single\"\"Quoted\'");
        token = tokenizer(token.end());
        TESTO_ENSURE(token == "\"not closed quotes\'");
        token = tokenizer(token.end());
        TESTO_ENSURE(!token);
    }
};

TESTO_ADD_TEST("Utils", TokenizerQuotes)
// ---------------------------------------------------------------------------
struct EscapeString{
    void invoke(){
        const char* correctString = "\"\'\n\r\t\f\x37\x18Прове\tрка и по\x17 русски\\Ерунда вот такая вот фигня-я-я";
        const char* begin = correctString;
        const char* end = correctString + strlen(correctString);

        MemoryWriter buf;
        escapeString(buf, begin, end);

        std::string unescapedString;
        const char* str = buf.c_str();
        unescapeString(unescapedString, str, str + buf.position());
        TESTO_ENSURE(unescapedString == correctString);

        const char* escapedString = "\\tTesti\\nng this\\ one\\";
        const char* reference = "\tTesti\nng this one";

        unescapeString(unescapedString, escapedString, escapedString + strlen(escapedString));
        TESTO_ENSURE(unescapedString == reference);
    }
};

TESTO_ADD_TEST("Utils", EscapeString)
// ---------------------------------------------------------------------------
struct MemoryWriterFloat{
    void invoke(){
        const float FLT_COMPARE_TOLERANCE = 1e-5f;
        {
            float initialValue = float(M_PI);
            MemoryWriter buf;
            buf << initialValue;

            float value = float(atof(buf.c_str()));
            TESTO_ENSURE(fabs(initialValue - value) < FLT_COMPARE_TOLERANCE);
        }


        {
            float initialValue = float(-M_PI);
            MemoryWriter buf;
            buf << initialValue;

            float value = float(atof(buf.c_str()));
            TESTO_ENSURE(fabs(initialValue - value) < FLT_COMPARE_TOLERANCE);
        }

        {
            float initialValue = float(1e+10f);
            MemoryWriter buf;
            buf << initialValue;

            float value = float(atof(buf.c_str()));
            TESTO_ENSURE(fabs(initialValue - value) < FLT_COMPARE_TOLERANCE);
        }

        {
            float initialValue = float(1e-10f);
            MemoryWriter buf;
            buf << initialValue;

            float value = float(atof(buf.c_str()));
            TESTO_ENSURE(fabs(initialValue - value) < FLT_COMPARE_TOLERANCE);
        }

        {
            float initialValue = float(1e-10f);
            MemoryWriter buf;
            buf << initialValue;

            float value = float(atof(buf.c_str()));
            TESTO_ENSURE(fabs(initialValue - value) < FLT_COMPARE_TOLERANCE);
        }
    }
};

TESTO_ADD_TEST("Utils", MemoryWriterFloat)

TESTO_END()
