#pragma once

#include "ww/_WidgetWithWindow.h"
#include "ww/Color.h"

namespace ww{

class ColorRampImpl;
class ColorRamp : public _WidgetWithWindow{
public:
	ColorRamp(int border = 0);

	sigslot::signal0& signalChanged(){ return signalChanged_; }

	void set(const Color& color);
	const Color& get() const{ return color_; }
protected:
	ColorRampImpl& impl();
	sigslot::signal0 signalChanged_;

	Color color_;
	friend class ColorRampImpl;
};

}

