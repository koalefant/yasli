/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <map>
#include "ww/API.h"
#include "ww/Signal.h"
#include "KeyPress.h"
#include "yasli/Pointers.h"

namespace ww{

class Application;
class HotkeyFilter;
class HotkeyContext : public RefCounter{
public:
    HotkeyContext();
    ~HotkeyContext();

	bool injectPress(KeyPress key);

	Signal<>& signalPressed(KeyPress key) { return signalsPressed_[key]; }
	Signal<KeyPress, bool&>& signalPressedAny(){ return signalPressedAny_; }

    void installFilter(Application* app);
    void uninstallFilter(Application* app);
protected:
	typedef std::map<KeyPress, Signal<>> HotkeySignals;
	HotkeySignals signalsPressed_;

	Signal<KeyPress, bool&> signalPressedAny_;
	yasli::SharedPtr<HotkeyFilter> filter_;
};

}

