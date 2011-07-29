#pragma once

#include <map>
#include "ww/API.h"
#include "ww/sigslot.h"
#include "KeyPress.h"
#include "yasli/Pointers.h"

namespace ww{

class Application;
class HotkeyFilter;
class WW_API HotkeyContext : public RefCounter{
public:
    HotkeyContext();
    ~HotkeyContext();

	bool injectPress(KeyPress key);

	signal0& signalPressed(KeyPress key) { return signalsPressed_[key]; }
	signal2<KeyPress, bool&>& signalPressedAny(){ return signalPressedAny_; }

    void installFilter(Application* app);
    void uninstallFilter(Application* app);
protected:
	typedef std::map<KeyPress, signal0> HotkeySignals;
	HotkeySignals signalsPressed_;

	signal2<KeyPress, bool&> signalPressedAny_;
    HotkeyFilter* filter_;
};

}

