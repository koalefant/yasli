#pragma once

#include "ww/_WidgetWithWindow.h"
#include "ww/Color.h"

namespace ww{

class ColorRectImpl;
class ColorRect : public _WidgetWithWindow{
public:
	ColorRect(Color color = Color(255, 255, 255), int border = 0);
	~ColorRect();

	void set(const Color& color);
	const Color& get() const{ return color_; }
	sigslot::signal0& signalActivate(){ return signalActivate_; }
protected:
	ColorRectImpl& impl();
	sigslot::signal0 signalActivate_;
	Color color_;
	friend class ColorRectImpl;
};

}

