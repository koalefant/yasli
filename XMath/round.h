#pragma once
#include <emmintrin.h>

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

#ifdef WIN32
inline int round(double x)
{
#ifdef _M_AMD64
	return _mm_cvtsd_si32(_mm_load_sd(&x));
#else
	int a;
	_asm {
        fld x
        fistp dword ptr a
    }
	return a;
#endif
}

inline int round(float x)
{
	return _mm_cvtt_ss2si(_mm_load_ss(&x));
}
#else
inline int round(float x)
{
  return lroundf(x);
}

/*
inline int round(double x)
{
  return lround(x);
}
*/
#endif
