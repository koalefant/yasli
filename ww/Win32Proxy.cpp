#include "StdAfx.h"
#include "ww/Win32Proxy.h"
#include "ww/_WidgetWindow.h"
#include "ww/HotkeyContext.h"
#include "ww/Win32/Rectangle.h"
#include "yasli/Archive.h"

namespace ww{

class Win32ProxyImpl : public _ContainerWindow{
public:
	Win32ProxyImpl(Win32Proxy* owner, HWND parent);
	const wchar_t* className() const{ return L"ww.Win32Proxy"; }
	BOOL onMessageSize(UINT type, USHORT width, USHORT height);

protected:
	Win32Proxy* owner_;
};

Win32ProxyImpl::Win32ProxyImpl(Win32Proxy* owner, HWND _parent)
: _ContainerWindow(owner)
, owner_(owner)
{
	Win32::Window32 parent(_parent);
	Win32::Rect rect;
	GetClientRect(_parent, &rect);
	VERIFY(create(L"", WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_CLIPCHILDREN, rect.recti(), parent));
	

	ASSERT(::IsWindow(handle_));
	::SetParent(handle_, _parent);
	ASSERT(::GetParent(handle_) == _parent);
	ASSERT(::IsWindow(handle_));
}

BOOL Win32ProxyImpl::onMessageSize(UINT type, USHORT width, USHORT height)
{
	if(!creating_){
		__super::onMessageSize(type, width, height);
		owner_->_arrangeChildren();
	}
	return true;
}

// ---------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
Win32Proxy::Win32Proxy(HWND parent, int border)
: _ContainerWithWindow(new Win32ProxyImpl(this, parentHwnd_ = parent), border)
{
	focusedWidget_ = 0;
	hotkeyContext_ = new HotkeyContext;
}
#pragma warning(pop)

Win32ProxyImpl& Win32Proxy::impl()
{
	return static_cast<Win32ProxyImpl&>(*_window());
}


void Win32Proxy::_arrangeChildren()
{
	RECT rect;
	VERIFY(::GetClientRect(parentHwnd_, &rect));
	VERIFY(::InflateRect(&rect, -border_, -border_));

	Rect recti(rect.left, rect.top, rect.right, rect.bottom);
	Container::_setPosition(recti);
	_window()->move(recti);

	if(child_)
		child_->_setPosition(recti);
}

void Win32Proxy::add(Widget* widget)
{
	if(child_)
		child_->_setParent(0);
	child_ = widget;
	child_->_setParent(this);
	_arrangeChildren();
}

void Win32Proxy::visitChildren(WidgetVisitor& visitor) const
{
	if(child_)
		visitor(*child_);
}

void Win32Proxy::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_STATE)){
		if(child_)
			ar(*child_, "widget", "Widget");
	}
}

}
