#include "StdAfx.h"
#include "Timer.h"
#include "ww/Win32/Window.h"
#include "ww/Widget.h"
#include "XMath/round.h"

namespace ww{

class TimerImpl : public Win32::TimerInterface{
public:
	TimerImpl(Timer* owner)
	: owner_(owner)
	{}
	void onTimer(){
		owner_->signalTimer().emit();
	}
protected:
	Timer* owner_;
};

Timer::Timer(Widget* owner, float interval)
{
	owner_ = owner;
	impl_ = new TimerImpl(this);
	owner->_window()->attachTimer(impl_, round(interval * 1000.0f));
}

Timer::~Timer()
{
	owner_->_window()->detachTimer(impl_);
	impl_ = 0;
}

}
