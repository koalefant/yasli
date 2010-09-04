#pragma once

#include "ww/Dialog.h"
#include "KeyPress.h"

namespace ww{

class HotkeyButton;

class HotkeyDialog : public Dialog{
public:
	HotkeyDialog(Widget* owner, KeyPress key = KeyPress());
	HotkeyDialog(HWND owner, KeyPress key = KeyPress());
	KeyPress& get(){ return key_; }

	void onChanged();

	void onKeyDefault();
	void onKeyCancel();
protected:
	HotkeyButton* button_;
	KeyPress key_;
};

}

