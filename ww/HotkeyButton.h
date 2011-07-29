#pragma once

#include "ww/_WidgetWithWindow.h"
#include "KeyPress.h"

namespace ww{

class HotkeyButtonImpl;
class HotkeyButton : public _WidgetWithWindow{
public:
	HotkeyButton(int border = 0);
	signal0& signalChanged(){ return signalChanged_; }

	KeyPress get() const{ return key_; }
protected:
	signal0 signalChanged_;
	HotkeyButtonImpl& impl();
	KeyPress key_;
	friend class HotkeyButtonImpl;
};

}

