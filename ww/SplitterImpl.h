#pragma once



#include "ww/Win32/Window.h"
#include "ww/Win32/Handle.h"
namespace ww{
class Splitter;
class SplitterImpl : public Win32::Window32{
public:
	const wchar_t* className() const{ return L"ww.SplitterImpl"; }
	SplitterImpl(ww::Splitter* owner);
protected:
	BOOL	onMessageSetCursor(HWND window, USHORT hitTest, USHORT message);
	void	onMessageLButtonDown(UINT button, int x, int y);
	void	onMessageLButtonUp(UINT button, int x, int y);
	void	onMessageMouseMove(UINT button, int x, int y);

	void	onMessagePaint();

	int splitterByPoint(Vect2i point);
	Recti getSplitterRect(int splitterIndex);

	ww::Splitter* owner_;
	Vect2i lastCursorPosition_;
	int grabbedSplitter_;
};
};

