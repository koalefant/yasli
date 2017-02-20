#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#undef min
#undef max

inline int min(int x,int y){ return x < y ? x : y; }
inline float min(float x,float y){ return x < y ? x : y; }
inline double min(double x,double y){ return x < y ? x : y; }

inline int max(int x,int y){ return x > y ? x : y; }
inline float max(float x,float y){ return x > y ? x : y; }
inline double max(double x,double y){ return x > y ? x : y; }

template <class T> inline T min(const T& a, const T& b, const T& c) { return min(min(a, b), c); }
template <class T> inline T max(const T& a, const T& b, const T& c) { return max(max(a, b), c); }

