#pragma once

#include <vector>

#include "ww/Win32/Window.h"
#include "ww/PropertyTree.h"
#include "ww/_WidgetWindow.h"

namespace ww{

class TreeImpl;

class DragWindow : public Win32::Window32
{
public:
	const wchar_t* className() const{ return L"ww.DragWindow"; }
	DragWindow(TreeImpl* treeImpl);

	void set(TreeImpl* treeImpl, PropertyRow* row, const Recti& rowRect);
	void show();
	void hide();
	void move(int deltaX, int deltaY);

	void onMessagePaint();
	BOOL onMessageEraseBkgnd(HDC dc);
protected:
	void setWindowPos(bool visible);
	void drawRow(HDC dc);
	void updateLayeredWindow();

	bool useLayeredWindows_;
	PropertyRow* row_;
	Recti rect_;
	TreeImpl* treeImpl_;
	Vect2i offset_;
	HBITMAP bitmap_;
	COLORREF* bitmapBits_;
};


class DragController
{
public:
	DragController(TreeImpl* treeImpl);
	void beginDrag(PropertyRow* row, POINT startPoint);
	bool dragOn(POINT screenPoint);
	void drop(POINT screenPoint);

	void drawUnder(HDC dc);
	void drawOver(HDC dc);
	void interrupt();
	void trackRow(POINT point);
	bool captured() const{ return captured_; }
protected:
	DragWindow window_;
	TreeImpl* treeImpl_;
	PropertyRow* row_;
	PropertyRow* hoveredRow_;
	PropertyRow* destinationRow_;
	POINT startPoint_;
	POINT lastPoint_;
	bool captured_;
	bool dragging_;
	bool before_;
};


class TreeImpl: public _ContainerWindow
{
public:
	enum TreeHitTest{
		TREE_HIT_PLUS,
		TREE_HIT_TEXT,
		TREE_HIT_ROW,
		TREE_HIT_NONE
	};

	TreeImpl(PropertyTree* owner);
	~TreeImpl();

	const wchar_t* className() const { return L"ww.TreeImpl"; }
	
	PropertyTree* tree() { return tree_; };
	PropertyTreeModel* model() { return tree_->model(); }

	void redraw(HDC dc);

	PropertyRow* rowByPoint(Vect2i point);
	TreeHitTest hitTest(PropertyRow* row, Vect2i pt, const Recti& rowRect);
	bool getRowRect(PropertyRow* row, Recti& rect, bool onlyVisible = true);
	bool toggleRow(PropertyRow* node);
	void ensureVisible(PropertyRow* row, bool update = true);

	void updateScrollBar();

	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);

	void onMessageMouseWheel(SHORT delta);
	void onMessageMouseMove(UINT button, int x, int y);

	void onMessageLButtonDblClk(int x, int y);
	void onMessageLButtonDown(UINT button, int x, int y);
	void onMessageLButtonUp(UINT button, int x, int y);
	void onMessageMButtonDown(UINT button, int x, int y);
	void onMessageRButtonDown(UINT button, int x, int y);

	BOOL onMessageEraseBkgnd(HDC dc);
	void onMessageScroll(UINT message, WORD type);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	BOOL onMessageSize(UINT type, USHORT width, USHORT height);

	int onMessageKillFocus(HWND newFocusedWindow);
	int onMessageSetFocus(HWND lastFocusedWindow);

	void onMessagePaint();

protected:
	Vect2i size_;
	Vect2i offset_;
	Recti area_;
	PropertyTree* tree_;
	PropertyRow* hoveredRow_;
	PropertyRow* capturedRow_;
	DragController drag_;
	bool redrawLock_;
	bool redrawRequest_;

	friend class PropertyTree;	
    friend class DragController;
};

}

