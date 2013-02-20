#pragma once

// Disable C++ RTTI use (e.g. typeid())
#define YASLI_NO_RTTI 1

// Disable extra struct-level for polymorphic pointer serialization
#define YASLI_NO_EXTRA_BLOCK_FOR_POINTERS 0

// Default filter for Archive: 0 - strict mode, -1 - silent mode.
#define YASLI_DEFAULT_FILTER 0
