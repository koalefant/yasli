#pragma once

#include "ww/_WidgetWithWindow.h"

namespace ww{
class SliderImpl;

class WW_API Slider : public _WidgetWithWindow{
public:
	Slider(int border = 0);

	void setValue(float value);
	float value() const{ return value_; }
	sigslot::signal0& signalChanged(){ return signalChanged_; }
	void setStepsCount(int stepsCount);
protected:
	sigslot::signal0 signalChanged_;
	float value_;
	int stepsCount_;

	SliderImpl& impl();
	friend class SliderImpl;
};
}

