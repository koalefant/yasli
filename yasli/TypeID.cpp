#include "TypeID.h"

namespace yasli {

YASLI_INLINE void TypeInfo::cleanTypeName(char*& d, const char* dend, const char*& s, const char* send)
{
	if(strncmp(s, "class ", 6) == 0)
		s += 6;
	else if(strncmp(s, "struct ", 7) == 0)
		s += 7;

	while(*s == ' ' && s != send)
		++s;

	if (s >= send)
		return;

	char* startd = d;
	while (d != dend && s != send) {
		while(*s == ' ' && s != send)
			++s;
		if (s == send)
			break;
		if (s >= send)
			break;
		if (*s == '<') {
			*d = '<';
			++d;
			++s;
			cleanTypeName(d, dend, s, send);
		}
		else if (*s == '>') {
			*d = '\0';
			return;
		}
		*d = *s;
		++s;
		++d;
	}
}

YASLI_INLINE void TypeInfo::extractTypeName(char *name, int nameLen, const char* funcName)
{
#ifdef __clang__
	const char* s;
	const char* send;
	const char* lastSquareBracket = strrchr(funcName, ']');
	const char* lastRoundBracket = strrchr(funcName, ')');
	if (lastSquareBracket && lastRoundBracket && lastSquareBracket > lastRoundBracket)
	{
		// static yasli::TypeID yasli::TypeID::get() [T = ActualTypeName]
		s = strstr(funcName, "[T = ");
		if (s)
			s += 5;
		send = strrchr(funcName, ']');
	}
	else
	{
		// old versions of Xcode
		// static yasli::funcNameHelper(ActualTypeName *)
		s = strstr(funcName, "(");
		if (s)
			s += 1;
		send = strstr(funcName, "*)");
		if (send > s && *(send-1) == ' ')
			--send;
	}
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)) // at least 4.4.0
	// static yasli::TypeID yasli::TypeID::get() [with T = ActualTypeName]
	const char* s = strstr(funcName, "[with T = ");
	if (s)
		s += 10;
	const char* send = strrchr(funcName, ']');
#else
	// static yasli::TypeID yasli::TypeID::get<ActualTypeName>()
	const char* s = strchr(funcName, '<');
	const char* send = strrchr(funcName, '>');
	YASLI_ASSERT(s != 0  && send != 0);
	if (s != send)
		++s;
#endif
	YASLI_ASSERT(s != 0  && send != 0);

	char* d = name;
	const char* dend = name + nameLen - 1;
	cleanTypeName(d, dend, s, send);
	*d = '\0';

	// This assertion is not critical, but may result in collision as
	// stripped name wil be used, e.g. for lookup in factory.
	YASLI_ASSERT(s == send && "Type name does not fit into the buffer");
}

YASLI_INLINE bool TypeID::operator==(const TypeID& rhs) const{
#if YASLI_NO_RTTI
	if (typeInfo_ == rhs.typeInfo_)
		return true;
	else if (!typeInfo_ || !rhs.typeInfo_)
		return false;
	else if (module_ == rhs.module_)
		return false;
	else
		return *typeInfo_ == *rhs.typeInfo_;
#else
	if(typeInfo_ && rhs.typeInfo_)
		return typeInfo_ == rhs.typeInfo_;
	else
	{
		const char* name1 = name();
		const char* name2 = rhs.name();
		return strcmp(name1, name2) == 0;
	}
#endif
}

YASLI_INLINE bool TypeID::operator!=(const TypeID& rhs) const{
	return !operator==(rhs);
}

YASLI_INLINE bool TypeID::operator<(const TypeID& rhs) const{
#if YASLI_NO_RTTI
	if (!typeInfo_)
		return rhs.typeInfo_ != 0;
	else if (!rhs.typeInfo_)
		return false;
	else
		return *typeInfo_ < *rhs.typeInfo_;
#else
	if(typeInfo_ && rhs.typeInfo_)
		return typeInfo_->before(*rhs.typeInfo_) > 0;
	else if(!typeInfo_)
		return rhs.typeInfo_!= 0;
	else if(!rhs.typeInfo_)
		return false;
	return false;
#endif
}

// ---------------------------------------------------------------------------

// We are trying to minimize type names here not to scare user of UI. Stripping
// namespaces, whitespaces and S/C/E/I prefixes.
YASLI_INLINE void makePrettyTypeName(char*& d, const char* dend, const char*& s, const char* send)
{
    if(strncmp(s, "class ", 6) == 0)
        s += 6;
    else if(strncmp(s, "struct ", 7) == 0)
        s += 7;

    while(*s == ' ' && s != send)
        ++s;

    // strip C/S/I/E prefixes
    if ((*s == 'C' || *s == 'S' || *s == 'I' || *s == 'E') && s[1] >= 'A' && s[1] <= 'Z')
        ++s;

    if (s >= send)
		return;

    char* startd = d;
    while (d != dend && s != send) {
        while(*s == ' ' && s != send)
            ++s;
        if (s == send)
            break;
        if (*s == ':' && s[1] == ':') {
            // strip namespaces
            s += 2;
            d = startd;

            if ((*s == 'C' || *s == 'S' || *s == 'I' || *s == 'E') && s[1] >= 'A' && s[1] <= 'Z')
                ++s;
        }
        if (s >= send)
            break;
        if (*s == '<') {
            *d = '<';
            ++d;
            ++s;
            makePrettyTypeName(d, dend, s, send);
        }
        else if (*s == '>') {
            *d = '\0';
			return;
        }
        *d = *s;
        ++s;
        ++d;
    }
}

YASLI_INLINE yasli::string makePrettyTypeName(const char* typeName) {
    char result[256] = "";
    char* d = result;
    const char* dend = d+sizeof(result)-1;
    const char* s = typeName;
    const char* send = typeName+strlen(typeName);
	makePrettyTypeName(d, dend, s, send);
	*d = '\0';
	return result;
}

}
