#include "StdAfx.h"
#include "ww/Slider.h"
#include "ww/_WidgetWindow.h"
#include "ww/Win32/MemoryDC.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Win32/Drawing.h"

namespace ww{

class SliderImpl : public _WidgetWindow{
public:
	SliderImpl(Slider* owner);
	const wchar_t* className()const{ return L"ww.Slider"; }

	void redraw(HDC dc);

	int onMessageSetFocus(HWND lastFocusedWindow);
	int onMessageKillFocus(HWND focusedWindow);
	void onMessageLButtonDown(UINT button, int x, int y);
	void onMessageLButtonUp(UINT button, int x, int y);
	void onMessageMouseMove(UINT button, int x, int y);
	void onMessagePaint();
	BOOL onMessageEraseBkgnd(HDC dc);
protected:
	Slider* owner_;
	bool dragging_;
};

SliderImpl::SliderImpl(Slider* owner)
: _WidgetWindow(owner)
, owner_(owner)
, dragging_(false)
{
	create(L"", WS_CHILD, Rect(0, 0, 20, 20), *Win32::_globalDummyWindow);
}

void SliderImpl::redraw(HDC dc)
{
	Win32::Rect rect;
	GetClientRect(*this, &rect);

	FillRect(dc, &rect, GetSysColorBrush(COLOR_BTNFACE));
	bool focused = GetFocus() == *this;
	if(focused)
		DrawFocusRect(dc, &rect);
	rect.left += 1;	rect.top += 1;
	rect.right -= 1; rect.bottom -= 1;
	drawSlider(dc, rect, owner_->value(), focused);
}

void SliderImpl::onMessagePaint()
{
	PAINTSTRUCT paintStruct;
	HDC dc = BeginPaint(*this, &paintStruct);
	{
		Win32::MemoryDC memoryDC(dc);
		redraw(memoryDC);
	}
	EndPaint(*this, &paintStruct);
}

BOOL SliderImpl::onMessageEraseBkgnd(HDC dc)
{
	return false;
}


int SliderImpl::onMessageSetFocus(HWND lastFocusedWindow)
{
	::RedrawWindow(*this, 0, 0, RDW_INVALIDATE);
	return __super::onMessageSetFocus(lastFocusedWindow);
}

int SliderImpl::onMessageKillFocus(HWND focusedWindow)
{
	::RedrawWindow(*this, 0, 0, RDW_INVALIDATE);
	return __super::onMessageKillFocus(focusedWindow);
}



void SliderImpl::onMessageMouseMove(UINT button, int x, int y)
{
	if((button & MK_LBUTTON) && dragging_){
		Win32::Rect rect;
		GetClientRect(*this, &rect);
		float value = clamp(float(x - 1) / float(rect.width() - 2), 0.0f, 1.0f);
		owner_->value_ = value;
		owner_->signalChanged_.emit();
		::RedrawWindow(*this, 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_NOERASE);
	}
	else
		dragging_ = false;
	__super::onMessageMouseMove(button, x ,y);
}

void SliderImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	Win32::Rect rect;
	GetClientRect(*this, &rect);
	float value = clamp(float(x - 1) / float(rect.width() - 2), 0.0f, 1.0f);
	owner_->value_ = value;
	owner_->signalChanged_.emit();
	::RedrawWindow(*this, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

	dragging_ = true;
	//SetFocus(*this);
	owner_->_setFocus();
	SetCapture(*this);
	__super::onMessageLButtonDown(button, x ,y);
}

void SliderImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	dragging_ = false;
	if(GetCapture() == *this)
		ReleaseCapture();
}

// ---------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
Slider::Slider(int border)
: _WidgetWithWindow(new SliderImpl(this), border)
, stepsCount_(0)
{
    _setMinimalSize(20, 20);
}
#pragma warning(pop)

SliderImpl& Slider::impl()
{
	return static_cast<SliderImpl&>(*__super::_window());
}

void Slider::setValue(float value)
{
	value_ = value;
	RedrawWindow(impl(), 0, 0, RDW_INVALIDATE);
}

void Slider::setStepsCount(int stepsCount)
{
	stepsCount_ = stepsCount;
}

}
