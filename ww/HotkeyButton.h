/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/_WidgetWithWindow.h"
#include "KeyPress.h"

namespace ww{

class HotkeyButtonImpl;
class HotkeyButton : public _WidgetWithWindow{
public:
	HotkeyButton(int border = 0);
	Signal<>& signalChanged(){ return signalChanged_; }

	KeyPress get() const{ return key_; }
protected:
	Signal<> signalChanged_;
	HotkeyButtonImpl& impl();
	KeyPress key_;
	friend class HotkeyButtonImpl;
};

}

