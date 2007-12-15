#pragma once

#include "UtilsAPI.h"

#include <string>
#include "MemoryWriter.h"

void UTILS_API escapeString(MemoryWriter& dest, const char* begin, const char* end);
void UTILS_API unescapeString(std::string& dest, const char* begin, const char* end);
