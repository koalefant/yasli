#pragma once

#include <xmmintrin.h>

#pragma warning(disable: 4725)

inline const float sqrtFast(const float x)
{
	float r;
	_mm_store_ss(&r, _mm_sqrt_ss(_mm_load_ss(&x)));
	return r;
}

inline float invSqrtFast(float x)
{
	x += 1e-7f; // ƒобавка, устран€юща€ деление на 0
	float r;
	_mm_store_ss(&r, _mm_rsqrt_ss(_mm_load_ss(&x)));
	return r;
}

// ¬ 3 раза быстрее за счет проверки аргументов, точна€.
inline float fmodFast(float a, float b)
{
	float result;
	_asm
	{
		fld b
			fld a
cycle_fast_fmod:
		fprem
			fnstsw ax
			sahf
			jp short cycle_fast_fmod
			fstp st(1)
			fstp result
	}
	return result;
}

inline unsigned int F2DW( float f ) 
{ 
	return *((unsigned int*)&f); 
}

inline float DW2F( unsigned int f ) 
{ 
	return *((float*)&f); 
}

