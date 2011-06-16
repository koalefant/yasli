#include "StdAfx.h"

#include "ww/ProgressBar.h"
#include "ww/_WidgetWindow.h"
#include <windowsx.h>
#include <CommCtrl.h>

#include "ww/Serialization.h"
#include "yasli/TypesFactory.h"

#define MAX_RANGE 10000

#pragma message("Automatically linking with comctl32.lib") 
#pragma comment(lib, "comctl32.lib") 

namespace ww{

YASLI_CLASS(Widget, ProgressBar, "ProgressBar");
#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

class ProgressBarImpl: public _WidgetWindow{
public:
	ProgressBarImpl(ProgressBar* owner);
	const wchar_t* className() const{ return PROGRESS_CLASS; }
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	void setProgressBarPosition(float pos);
protected:
	ww::ProgressBar* owner_;
	static WNDPROC controlWindowProc_;
};

WNDPROC ProgressBarImpl::controlWindowProc_ = 0;

#pragma warning(push)
#pragma warning(disable: 4312) // 'type cast' : conversion from 'LONG' to 'HINSTANCE' of greater size

ProgressBarImpl::ProgressBarImpl(ProgressBar* owner)
: _WidgetWindow(owner)
, owner_(owner)
{
	InitCommonControls();
	VERIFY(create(L"", WS_CHILD | PBS_SMOOTH | WS_VISIBLE | WS_CLIPCHILDREN, Rect(0, 0, 42, 42), *Win32::_globalDummyWindow));
	controlWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(handle_, GWLP_WNDPROC));
	::SetWindowLongPtr(handle_, GWLP_WNDPROC, reinterpret_cast<LONG>(&Win32::universalWindowProcedure));
	::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG>(this));
	SendMessage(handle_, PBM_SETRANGE, 0, MAKELPARAM(0, MAX_RANGE)); 
	SetWindowFont(handle_, Win32::defaultFont(), FALSE);
}

void ProgressBarImpl::setProgressBarPosition(float pos)
{
	SendMessage(handle_, PBM_SETPOS, (int)(pos * MAX_RANGE), 0); 	
}

#pragma warning(pop)

LRESULT ProgressBarImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	return ::CallWindowProc(controlWindowProc_, handle_, message, wparam, lparam);
}

// --------------------------------------------------------------------------------------------

ProgressBar::ProgressBar(int border)
: _WidgetWithWindow(new ProgressBarImpl(this), border)
{
    _setMinimalSize(Vect2i(4, 4));
    setRequestSize(Vect2i(24, 24));
    pos_ = 0.f;
    setPosition(pos_);
}

void ProgressBar::serialize(Archive& ar)
{
    if(ar.filter(SERIALIZE_DESIGN)){
        ar.serialize(pos_, "pos", 0);
    }
    Widget::serialize(ar);
}

void ProgressBar::setPosition(float pos)
{
    ASSERT((pos >= 0) && (pos <= 1));
    pos_ = pos;
    window()->setProgressBarPosition(pos_);
}
};


#pragma warning(pop)

