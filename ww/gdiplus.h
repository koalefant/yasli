#pragma once

#include <algorithm>
using std::min;
using std::max;
#include <rpc.h>
#include <unknwn.h>
#include <gdiplus.h>
#include "XMath/Recti.h"

inline Gdiplus::Color gdiplusSysColor(int index)
{
	Gdiplus::Color result;
	result.SetFromCOLORREF(GetSysColor(index));
	return result;
}

inline Gdiplus::Rect gdiplusRect(const RECT& rect)
{
    return Gdiplus::Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
}

inline Gdiplus::Rect gdiplusRect(const Recti& rect)
{
    return Gdiplus::Rect(rect.left(), rect.top(), rect.width(), rect.height());
}

inline Gdiplus::RectF gdiplusRectF(const RECT& rect)
{
	return Gdiplus::RectF(Gdiplus::REAL(rect.left), Gdiplus::REAL(rect.top), Gdiplus::REAL(rect.right - rect.left), Gdiplus::REAL(rect.bottom - rect.top));
}

inline Gdiplus::RectF gdiplusRectF(const Recti& rect)
{
	return Gdiplus::RectF(Gdiplus::REAL(rect.left()), Gdiplus::REAL(rect.top()), Gdiplus::REAL(rect.width()), Gdiplus::REAL(rect.height()));
}

inline Gdiplus::RectF gdiplusRectF(const Gdiplus::Rect& rect)
{
	return Gdiplus::RectF(Gdiplus::REAL(rect.X), Gdiplus::REAL(rect.Y), Gdiplus::REAL(rect.Width), Gdiplus::REAL(rect.Height));
}

