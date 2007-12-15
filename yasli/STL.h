#pragma once

#include <vector>

#include "yasli/Serializer.h"

class Archive;

template<class T, class Alloc>
bool serialize(Archive& ar, std::vector<T, Alloc>& container, const char* name);
