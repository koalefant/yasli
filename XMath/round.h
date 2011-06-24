#pragma once

#ifdef _WIN64
#include <emmintrin.h>
#endif

#pragma warning (disable : 4244)

template<class T, class T1, class T2> 
inline T clamp(const T& x, const T1& xmin, const T2& xmax)
{
  if(x < xmin)
    return xmin;
  if(x > xmax)
    return xmax;
  return x;
}

inline int round(double x)
{
    int a;
#ifdef _WIN64
	return _mm_cvtsd_si32(_mm_load_sd(&x));
#else
    _asm {
        fld x
        fistp dword ptr a
    }
#endif
    return a;
}

inline int round(float x)
{
#ifdef _WIN64
	return _mm_cvtt_ss2si(_mm_load_ss(&x));
#else
    int a;
    _asm {
        fld x
        fistp dword ptr a
    }
    return a;
#endif
}

