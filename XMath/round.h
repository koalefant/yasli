#pragma once

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
    _asm {
        fld x
        fistp dword ptr a
    }
    return a;
}

inline int round(float x)
{
    int a;
    _asm {
        fld x
        fistp dword ptr a
    }
    return a;
}

