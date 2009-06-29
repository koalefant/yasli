#pragma once

#include "API.h"

#include <string>
#include "MemoryWriter.h"

namespace yasli{
void YASLI_API escapeString(MemoryWriter& dest, const char* begin, const char* end);
void YASLI_API unescapeString(std::string& dest, const char* begin, const char* end);
}
