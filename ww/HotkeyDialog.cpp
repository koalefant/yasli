#include "StdAfx.h"
#include "ww/HotkeyDialog.h"
#include "ww/HotkeyButton.h"

#include "ww/Win32/Window.h"

namespace ww{

HotkeyDialog::HotkeyDialog(Widget* owner, KeyPress key)
: Dialog(owner, 0)
, button_(0)
{
	setMinimizeable(false);
	setResizeable(false);
	setShowTitleBar(false);
	button_ = new HotkeyButton(0);
	button_->signalChanged().connect(this, &HotkeyDialog::onChanged);
	add(button_, PACK_FILL);
}

HotkeyDialog::HotkeyDialog(HWND owner, KeyPress key)
: Dialog(owner, 0)
, button_(0)
{
	// COPY_PASTE
	setMinimizeable(false);
	setResizeable(false);
	setShowTitleBar(false);
	button_ = new HotkeyButton(0);
	button_->signalChanged().connect(this, &HotkeyDialog::onChanged);
	add(button_, PACK_FILL);
}

void HotkeyDialog::onChanged()
{
	key_ = button_->get();
	Dialog::onResponse(RESPONSE_OK);
}

void HotkeyDialog::onKeyDefault()
{
	// не вызываем __super::onKeyDefault
}

void HotkeyDialog::onKeyCancel()
{
	// не вызываем __super::onKeyCancel
}

}
