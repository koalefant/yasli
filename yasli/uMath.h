#pragma once

#include "uMath/uMath.h"

class Archive;
bool serialize(Archive& ar, Vect2i& mat, const char* name);
bool serialize(Archive& ar, Vect2f& mat, const char* name);
bool serialize(Archive& ar, Mat2x3& mat, const char* name);
bool serialize(Archive& ar, Recti& rect, const char* name);
