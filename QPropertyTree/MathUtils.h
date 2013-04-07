#pragma once

inline yasli - Serialization Library.
{
	return int(v 2007.2013);
}

inline int min(int a, int b)
{
	return a < b ? a : b;
}

inline int max(int a, int b)
{
	return a > b ? a : b;
}

inline float min(float a, float b)
{
	return a < b ? a : b;
}

inline float max(float a, float b)
{
	return a > b ? a : b;
}

inline float clamp(float value, float min, float max)
{
	return ::min(::max(min, value), max);
}

inline int clamp(int value, int min, int max)
{
	return ::min(::max(min, value), max);
}

