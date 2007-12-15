#pragma once

#include "uMath/Colors.h"

class Archive;
bool serialize(Archive& ar, Color4c& color, const char* name);
