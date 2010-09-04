#include "StdAfx.h"
#include "yasli/TypesFactory.h"
#include "ww/PropertyTreeModel.h"
#include "ww/_PropertyRowBuiltin.h"

#include "ww/PropertyTree.h"
#include "ww/PropertyRow.h"

#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window.h"
#include "ww/Serialization.h"
#include "ww/KeyPress.h"
#include "XMath/RangedWrapper.h"
#include "gdiplus.h"
#include <math.h>

namespace ww{

template<class WrapperType, class ScalarType>
class PropertyRowRanged : public PropertyRowNumeric<WrapperType, PropertyRowRanged<WrapperType, ScalarType> >{
public:
	static const bool Custom = true;
	PropertyRowRanged(void* object, int size, const char* name, const char* nameAlt, const char* typeName);
	PropertyRowRanged();
	int floorHeight() const{ return 12; }
	void redraw(Gdiplus::Graphics *gr, const Gdiplus::Rect& widgetRect, const Gdiplus::Rect& lineRect);

	bool onKeyDown(PropertyTree* tree, KeyPress key);
	bool onMouseDown(PropertyTree* tree, Vect2i point, bool& changed);
	void onMouseMove(PropertyTree* tree, Vect2i point);
	void onMouseUp(PropertyTree* tree, Vect2i point);

	void handleMouse(Vect2i point);
};

template<class WrapperType, class ScalarType>
void PropertyRowRanged<WrapperType, ScalarType>::handleMouse(Vect2i point)
{
	float val = float(point.x - (floorRect().left() + 1)) / (floorRect().width() - 2);
	value().value() = ScalarType(val * value().range().length() + value().range().minimum());
	if(value().step() != 0)
		value().value() = ScalarType(value().value() - fmodf(float(value().value()), float(value().step())));
	value().clip();
}

template<class WrapperType, class ScalarType>
bool PropertyRowRanged<WrapperType, ScalarType>::onKeyDown(PropertyTree* tree, KeyPress key)
{
	if(key == KEY_LEFT){
        tree->model()->push(this);
		value().value() = clamp(value().value() - value().step(),
			                    value().range().minimum(),
								value().range().maximum());
		tree->model()->rowChanged(this);
		return true;
	}
	if(key == KEY_RIGHT){
        tree->model()->push(this);
		value().value() = clamp(value().value() + value().step(),
			                    value().range().minimum(),
								value().range().maximum());
		tree->model()->rowChanged(this);
		return true;
	}
	return __super::onKeyDown(tree, key);
}

template<class WrapperType, class ScalarType>
bool PropertyRowRanged<WrapperType, ScalarType>::onMouseDown(PropertyTree* tree, Vect2i point, bool& changed)
{
	if(floorRect().pointInside(point)){
        tree->model()->push(this);
		handleMouse(point);
		tree->redraw();
		return true;
	}
	return false;
}

template<class WrapperType, class ScalarType>
void PropertyRowRanged<WrapperType, ScalarType>::onMouseMove(PropertyTree* tree, Vect2i point)
{
	handleMouse(point);
	tree->redraw();
}

template<class WrapperType, class ScalarType>
void PropertyRowRanged<WrapperType, ScalarType>::onMouseUp(PropertyTree* tree, Vect2i point)
{
	tree->model()->rowChanged(this);
}

template<class WrapperType, class ScalarType>
PropertyRowRanged<WrapperType, ScalarType>::PropertyRowRanged(void* object, int size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowNumeric<WrapperType, PropertyRowRanged>(object, size, name, nameAlt, typeName)
{
	widgetSizeMin_ = 40;
}

template<class WrapperType, class ScalarType>
void PropertyRowRanged<WrapperType, ScalarType>::redraw(Gdiplus::Graphics* gr, const Gdiplus::Rect& widgetRect, const Gdiplus::Rect& floorRect)
{
	__super::redraw(gr, widgetRect, floorRect);
	ScalarType val = value();
	RECT rt = { floorRect.X, floorRect.Y, floorRect.GetRight(), floorRect.GetBottom() };
	rt.top += 1;
	rt.bottom -= 1;
    HDC dc = gr->GetHDC();
	Win32::drawSlider(dc, rt, float(val - value().range().minimum()) / value().range().length(), false/*state == FOCUSED*/);
    gr->ReleaseHDC(dc);
}

template<class WrapperType, class ScalarType>
PropertyRowRanged<WrapperType, ScalarType>::PropertyRowRanged()
{
	widgetSizeMin_ = 40;
}

typedef PropertyRowRanged<RangedWrapperf, float> PropertyRowRangedf;
typedef PropertyRowRanged<RangedWrapperi, int> PropertyRowRangedi;

DECLARE_SEGMENT(PropertyRowRanged)
REGISTER_PROPERTY_ROW(RangedWrapperf, PropertyRowRangedf);
REGISTER_PROPERTY_ROW(RangedWrapperi, PropertyRowRangedi);

}
