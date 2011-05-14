#pragma once

#include "ww/API.h"
#include "ww/sigslot.h"
#include "yasli/Pointers.h"

namespace ww{
class Widget;
class TimerImpl;
class Timer : public RefCounter{
public:
	Timer(Widget* owner, float interval = 0.1f);
	virtual ~Timer();
	sigslot::signal0& signalTimer(){ return signalTimer_; }
protected:
	sigslot::signal0 signalTimer_;
	yasli::SharedPtr<TimerImpl> impl_;
    Widget* owner_;
};

}

