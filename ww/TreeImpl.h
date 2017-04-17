/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>

#include "ww/Win32/Window32.h"
#include "ww/PropertyTree.h"
#include "ww/_WidgetWindow.h"

namespace ww{

class TreeImpl;

class DragWindow : public Win32::Window32
{
public:
	DragWindow(TreeImpl* treeImpl);

	void set(TreeImpl* treeImpl, PropertyRow* row, const Rect& rowRect);
	void show();
	void hide();
	void move(int deltaX, int deltaY);

	void onMessagePaint();
	BOOL onMessageEraseBkgnd(HDC dc);
protected:
	void setWindowPos(bool visible);
	void drawRow(HDC dc);
	void updateLayeredWindow();

	PropertyRow* row_;
	Rect rect_;
	TreeImpl* treeImpl_;
	Vect2 offset_;
	HBITMAP bitmap_;
	COLORREF* bitmapBits_;
};


class DragController
{
public:
	DragController(TreeImpl* treeImpl);
	void beginDrag(PropertyRow* clickedRow, PropertyRow* draggedRow, POINT startPoint);
	bool dragOn(POINT screenPoint);
	void drop(POINT screenPoint);

	void drawUnder(HDC dc);
	void drawOver(HDC dc);
	void interrupt();
	void trackRow(POINT point);
	bool captured() const{ return captured_; }
	bool isDragging() const{ return dragging_; }
	PropertyRow* draggedRow() const{ return row_; }
protected:
	DragWindow window_;
	TreeImpl* treeImpl_;
	PropertyRow* row_;
	PropertyRow* clickedRow_;
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

	PropertyTree* tree() { return tree_; };
	PropertyTreeModel* model() { return tree_->model(); }

	Vect2 pointToRootSpace(Vect2 pointInWindowSpace) const;
	PropertyRow* rowByPoint(Vect2 pointInWindowSpace);

	TreeHitTest hitTest(PropertyRow* row, Vect2 pointInWindowSpace, const Rect& rowRect);

	bool getRowRect(PropertyRow* row, Rect& rectInWindowSpace, bool onlyVisible = true);
	bool toggleRow(PropertyRow* node);
	void ensureVisible(PropertyRow* row, bool update = true);

    void updateArea();
	void updateScrollBar();

	int onMessageGetDlgCode(int keyCode, MSG* msg) override;
	int onMessageChar(UINT code, USHORT count, USHORT flags) override;
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags) override;

	void onMessageMouseWheel(SHORT delta) override;
	void onMessageMouseMove(UINT button, int x, int y) override;

	void onMessageLButtonDblClk(int x, int y) override;
	void onMessageLButtonDown(UINT button, int x, int y) override;
	void onMessageLButtonUp(UINT button, int x, int y) override;
	void onMessageMButtonDown(UINT button, int x, int y) override;
	void onMessageRButtonDown(UINT button, int x, int y) override;

	BOOL onMessageEraseBkgnd(HDC dc) override;
	void onMessageScroll(UINT message, WORD type) override;
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam) override;
	BOOL onMessageSize(UINT type, USHORT width, USHORT height) override;

	int onMessageKillFocus(HWND newFocusedWindow) override;
	int onMessageSetFocus(HWND lastFocusedWindow) override;

	void onMessagePaint() override;

protected:
	Vect2 size_;
	Vect2 offset_;
	Rect area_;
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

