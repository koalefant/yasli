#include "StdAfx.h"
#include "yasli/TypesFactory.h"
#include "ww/PropertyTreeModel.h"
#include "ww/_PropertyRowBuiltin.h"

#include "ww/PropertyTree.h"
#include "ww/TreeImpl.h"
#include "ww/PropertyRow.h"
#include "ww/PropertyDrawContext.h"

#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window.h"
#include "ww/Serialization.h"
#include "ww/KeyPress.h"
#include "ww/SliderDecorator.h"
#include "gdiplus.h"
#include <math.h>

namespace ww{

template<class WrapperType, class ScalarType>
class PropertyRowSlider : public PropertyRowNumeric<WrapperType, PropertyRowSlider<WrapperType, ScalarType> >{
public:
	static const bool Custom = true;
	PropertyRowSlider(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName);
	PropertyRowSlider();
	int floorHeight() const{ return 12; }
	void redraw(const PropertyDrawContext& context);

	bool onKeyDown(PropertyTree* tree, KeyPress key);
	bool onMouseDown(PropertyTree* tree, Vect2 point, bool& changed);
	void onMouseMove(PropertyTree* tree, Vect2 point);
	void onMouseUp(PropertyTree* tree, Vect2 point);

	void handleMouse(Vect2 point);
};

template<class WrapperType, class ScalarType>
void PropertyRowSlider<WrapperType, ScalarType>::handleMouse(Vect2 point)
{
	float val = float(point.x - (floorRect().left() + 1)) / (floorRect().width() - 2);
	value().value() = ScalarType(val * (value().maxValue() - value().minValue()) + value().minValue());
	if(value().step() != 0)
		value().value() = ScalarType(value().value() - fmodf(float(value().value()), float(value().step())));
	value().clip();
}

template<class WrapperType, class ScalarType>
bool PropertyRowSlider<WrapperType, ScalarType>::onKeyDown(PropertyTree* tree, KeyPress key)
{
	if(key == KEY_LEFT){
        tree->model()->push(this);
		value().value() = clamp(value().value() - value().step(),
			                    value().minValue(),
								value().maxValue());
		tree->model()->rowChanged(this);
		return true;
	}
	if(key == KEY_RIGHT){
        tree->model()->push(this);
		value().value() = clamp(value().value() + value().step(),
			                    value().minValue(),
								value().maxValue());
		tree->model()->rowChanged(this);
		return true;
	}
	return __super::onKeyDown(tree, key);
}

template<class WrapperType, class ScalarType>
bool PropertyRowSlider<WrapperType, ScalarType>::onMouseDown(PropertyTree* tree, Vect2 point, bool& changed)
{
	if(floorRect().pointInside(point)){
        tree->model()->push(this);
		handleMouse(point);
		tree->redraw();
		::SetCapture(tree->impl()->get());
		return true;
	}
	return false;
}

template<class WrapperType, class ScalarType>
void PropertyRowSlider<WrapperType, ScalarType>::onMouseMove(PropertyTree* tree, Vect2 point)
{
	handleMouse(point);
	tree->redraw();
}

template<class WrapperType, class ScalarType>
void PropertyRowSlider<WrapperType, ScalarType>::onMouseUp(PropertyTree* tree, Vect2 point)
{
	tree->model()->rowChanged(this);
	if(::GetCapture() == tree->impl()->get())
		::ReleaseCapture();
}

template<class WrapperType, class ScalarType>
PropertyRowSlider<WrapperType, ScalarType>::PropertyRowSlider(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName)
: PropertyRowNumeric<WrapperType, PropertyRowSlider>(object, size, name, nameAlt, typeName)
{
	widgetSizeMin_ = 40;
}

template<class WrapperType, class ScalarType>
void PropertyRowSlider<WrapperType, ScalarType>::redraw(const PropertyDrawContext& context)
{
	__super::redraw(context);
	ScalarType val = value();
	RECT rt = context.lineRect;
	rt.top += 1;
	rt.bottom -= 1;
    HDC dc = context.graphics->GetHDC();
	Win32::drawSlider(dc, rt, float(val - value().minValue()) / (value().maxValue() - value().minValue()), false/*state == FOCUSED*/);
    context.graphics->ReleaseHDC(dc);
}

template<class WrapperType, class ScalarType>
PropertyRowSlider<WrapperType, ScalarType>::PropertyRowSlider()
{
	widgetSizeMin_ = 40;
}

typedef PropertyRowSlider<SliderDecoratorf, float> PropertyRowSliderf;
typedef PropertyRowSlider<SliderDecoratori, int> PropertyRowSlideri;

DECLARE_SEGMENT(PropertyRowSlider)
REGISTER_PROPERTY_ROW(SliderDecoratorf, PropertyRowSliderf);
REGISTER_PROPERTY_ROW(SliderDecoratori, PropertyRowSlideri);

}
