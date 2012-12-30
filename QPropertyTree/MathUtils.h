#pragma once

inline int round(float v)
{
	return int(v + 0.5f);
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

inline int clamp(int value, int min, int max)
{
	return ::min(::max(min, value), max);
}

