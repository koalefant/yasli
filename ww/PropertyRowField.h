#pragma once
#include "PropertyRow.h"

namespace ww{

class PropertyRowField : public PropertyRow
{
public:
    PropertyRowField();
    PropertyRowField(const char* name, const char* nameAlt, const char* typeName);
    PropertyRowField(const char* name, const char* nameAlt, const Serializer& ser);

    void redraw(Gdiplus::Graphics* gr, const Gdiplus::Rect& widgetRect, const Gdiplus::Rect& floorRect);
};

}