#include "StdAfx.h"

#include "ww/Button.h"
#include "ww/_WidgetWindow.h"
#include "ww/Serialization.h"
#include "ww/Unicode.h"
#include "yasli/TypesFactory.h"

#define WIN32_LEAN_AND_MEAN
#include <windowsx.h>

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

namespace ww{
YASLI_CLASS(Widget, Button, "Button");

class ButtonImpl: public _WidgetWindow{
public:
	ButtonImpl(ww::Button* owner);
	const wchar_t* className() const{ return L"ww.ButtonOwner"; }
	
	BOOL onMessageWindowPosChanged(WINDOWPOS *windowPos);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	void setButtonText(const wchar_t* text);
	void setDefaultButton(bool defaultBtn);
protected:
	ww::Button* owner_;
	HWND button_;
};

#pragma warning(push)
#pragma warning(disable: 4312) // 'type cast' : conversion from 'LONG' to 'HINSTANCE' of greater size

static WNDPROC buttonWindowProc_ = 0;

LRESULT CALLBACK buttonWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	if(message == WM_KEYDOWN)
	{
		SendMessage(::GetParent(handle), WM_KEYDOWN, wparam, lparam);
		/*
		UINT key = UINT(wparam);
		if(key == VK_RETURN)
			SendMessageA(::GetParent(handle), WM_COMMAND, MAKEWPARAM(0, BN_CLICKED), (LONG) handle);
			*/
	}

	return ::CallWindowProc(buttonWindowProc_, handle, message, wparam, lparam);
}

ButtonImpl::ButtonImpl(ww::Button* owner)
: _WidgetWindow(owner)
, owner_(owner)
, button_(0)
{
	VERIFY(create(L"", WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN , Rect(0, 0, 42, 42), *Win32::_globalDummyWindow));
	button_ = ::CreateWindow( L"BUTTON", L"Unnamed", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 0, 0, 42, 42, *this, 0, (HINSTANCE)GetWindowLong(*this, GWLP_HINSTANCE), 0);
	ASSERT(button_);
	buttonWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(button_, GWLP_WNDPROC));
	::SetWindowLongPtr(button_, GWLP_WNDPROC, reinterpret_cast<LONG>(&buttonWindowProcedure));
	::SetWindowLongPtr(button_, GWLP_USERDATA, reinterpret_cast<LONG>(this));
	SetWindowFont(button_, Win32::defaultFont(), FALSE);
}

#pragma warning(pop)

void ButtonImpl::setDefaultButton(bool defaultBtn)
{
	if(defaultBtn)
		SendMessageA(button_, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE); 
	else
		SendMessageA(button_, BM_SETSTYLE, BS_PUSHBUTTON, TRUE); 
}

void ButtonImpl::setButtonText(const wchar_t* text)
{
	ESCAPE(::IsWindow(button_), return);
	VERIFY(::SetWindowText(button_, text));

	HFONT font = GetWindowFont(button_);
	Vect2i textSize = Win32::calculateTextSize(button_, font, text);

	Vect2i extraSize(8, 8);
	Vect2i minimalSize = textSize + Vect2i(owner_->border(), owner_->border()) * 2 + Vect2i(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER)) * 2 + extraSize;
	owner_->_setMinimalSize(minimalSize);
	owner_->_queueRelayout();
}


BOOL ButtonImpl::onMessageWindowPosChanged(WINDOWPOS *windowPos)
{
	if(button_)
	{
		int cx = windowPos->cx;
		int cy = windowPos->cy;
		::SetWindowPos(button_, 0, 0, 0, cx, cy, 0);
	}
	return FALSE;
}

LRESULT ButtonImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message){
	case WM_SIZE:
		{
			if(button_){
				UINT width = LOWORD(lparam);
				UINT height = HIWORD(lparam);
				::MoveWindow(button_, 0, 0, width, height, TRUE);
			}
			return TRUE;
		}
	case WM_ENABLE:
		{
			BOOL enabled = BOOL(wparam);
			if(button_){
				::EnableWindow(button_, enabled);
				ASSERT(::IsWindowEnabled(button_) == enabled);
			}
			return TRUE;
		}
	case WM_SETFOCUS:
		{
			::SetFocus(button_);
		}
		/*
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)(wparam);
			//::ExcludeClipRect(dc, 0, 0, 
			return FALSE;
		}*/
	case WM_COMMAND:
		if(HWND(lparam) == button_){
			switch(HIWORD(wparam)){
			case BN_CLICKED:
			case BN_PUSHED:
				owner_->onPressed();
				return TRUE;
			case BN_SETFOCUS:
                owner_->_setFocus();
				return TRUE;
			}
		}
	}
	return __super::onMessage(message, wparam, lparam);
}


// --------------------------------------------------------------------------------------------

Button::Button(const char* text, int border)
: _WidgetWithWindow(new ButtonImpl(this), border)
{
	_setMinimalSize(Vect2i(24, 24));
	setText(text);
	defaultBtn_ = false;
}

void Button::setDefaultFrame(bool enable)
{
	defaultBtn_ = enable;
	window()->setDefaultButton(defaultBtn_);
}

void Button::setText(const char* text)
{
	text_ = text;
	window()->setButtonText(toWideChar(text).c_str());
}

void Button::onPressed()
{
	signalPressed_.emit();
}

void Button::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_DESIGN)){
		ar.serialize(text_, "text", "&Текст");
	}
	Widget::serialize(ar);
}

};


#pragma warning(pop)

