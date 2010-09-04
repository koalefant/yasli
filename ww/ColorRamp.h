#pragma once

#include "ww/_WidgetWithWindow.h"
#include "XMath/Colors.h"
namespace ww{

class ColorRampImpl;
class ColorRamp : public _WidgetWithWindow{
public:
	ColorRamp(int border = 0);

	sigslot::signal0& signalChanged(){ return signalChanged_; }

	void set(const Color4f& color);
	const Color4f& get() const{ return color_; }
protected:
	ColorRampImpl& impl();
	sigslot::signal0 signalChanged_;

	Color4f color_;
	friend class ColorRampImpl;
};

}

