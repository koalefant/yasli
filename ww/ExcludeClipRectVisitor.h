#pragma once

#include "ww/Widget.h"
#include "ww/Win32/Window.h"
#include "ww/Win32/Rectangle.h"

struct ExcludeClipRectVisitor : public ww::WidgetVisitor{
	HDC dc_;
	ExcludeClipRectVisitor(HDC dc)
	: dc_(dc)
	{
	}

	void operator()(ww::Widget& widget){
		if(Win32::Window32* window = widget._window()){
			if(widget._visible() && widget._visibleInLayout() && ::IsWindowVisible(*window)){
				Win32::Rect rect;
				::GetClientRect(*window, &rect);
				window->clientToScreen(rect);
				window->parent()->screenToClient(rect);
				::ExcludeClipRect(dc_, rect.left, rect.top, rect.right, rect.bottom);
			}
		}
		else
			widget.visitChildren(*this);
	}
};

