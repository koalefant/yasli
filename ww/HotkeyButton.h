#pragma once

#include "ww/_WidgetWithWindow.h"
#include "KeyPress.h"

namespace ww{

class HotkeyButtonImpl;
class HotkeyButton : public _WidgetWithWindow{
public:
	HotkeyButton(int border = 0);
	sigslot::signal0& signalChanged(){ return signalChanged_; }

	KeyPress get() const{ return key_; }
protected:
	sigslot::signal0 signalChanged_;
	HotkeyButtonImpl& impl();
	KeyPress key_;
	friend class HotkeyButtonImpl;
};

}

