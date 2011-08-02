/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <string>
#include <algorithm>
#include "ww/TreeImpl.h"
#include "ww/Window.h"
#include "ww/Entry.h"

#include "ww/_WidgetWindow.h"
#include "ww/Win32/Handle.h"
#include "ww/Win32/MemoryDC.h"
#include "ww/Win32/Rectangle.h"

#include "ww/Serialization.h"
#include "yasli/TypesFactory.h"
#include "ww/ImageStore.h"
#include "ww/Unicode.h"
#include "ww/Win32/Window.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>
#include "gdiplus.h"
#include "PropertyDrawContext.h"
#include "PropertyRow.h"
#include "PropertyTree.h"
#include "PropertyTreeModel.h"

using namespace Gdiplus;

namespace ww{

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
DragWindow::DragWindow(TreeImpl* treeImpl)
: useLayeredWindows_(true)
, treeImpl_(treeImpl)
, offset_(0, 0)
{
	DWORD exStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	if(useLayeredWindows_){
		exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
		WW_VERIFY(create(L"", WS_POPUP | WS_DISABLED, Rect(0, 0, 320, 320), 0, exStyle));
	}
	else{
		Rect treeRect(0, 0, 100, 100);
		WW_VERIFY(create(L"", WS_POPUP | WS_DISABLED, treeRect, 0, exStyle));
	}

}
#pragma warning(pop)

void DragWindow::set(TreeImpl* treeImpl, PropertyRow* row, const Rect& rowRect)
{
	RECT rect;
	GetClientRect(treeImpl_->handle(), &rect);
	ClientToScreen(treeImpl_->handle(), (POINT*)&rect);
	ClientToScreen(treeImpl_->handle(), (POINT*)&rect + 1);

	offset_.x = rect.left;
	offset_.y = rect.top;

	row_ = row;
	rect_ = rowRect;
}

void DragWindow::setWindowPos(bool visible)
{
	SetWindowPos(handle(), HWND_TOP, rect_.left() + offset_.x,  rect_.top() + offset_.y,
				 rect_.width(), rect_.height(), 
				 SWP_NOOWNERZORDER | (visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW)| SWP_NOACTIVATE);
}

void DragWindow::show()
{
	if(useLayeredWindows_){
		updateLayeredWindow();
		setWindowPos(true);
		updateLayeredWindow();
	}
	else{
		setWindowPos(true);
	}
}

void DragWindow::move(int deltaX, int deltaY)
{
	offset_ += Vect2(deltaX, deltaY);
	setWindowPos(::IsWindowVisible(handle()) ? true : false);
}

void DragWindow::hide()
{
	setWindowPos(false);
}

struct DrawRowVisitor
{
	DrawRowVisitor(HDC dc) : dc_(dc) {}

	ScanResult operator()(PropertyRow* row, PropertyTree* tree)
	{
		if(row->pulledUp())
			row->drawRow(dc_, tree);

		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	HDC dc_;
};

void DragWindow::drawRow(HDC dc)
{
	Rect entireRowRect(0, 0, rect_.width(), rect_.height());

	HGDIOBJ oldBrush = ::SelectObject(dc, GetSysColorBrush(COLOR_BTNFACE));
	HGDIOBJ oldPen = ::SelectObject(dc, GetStockObject(BLACK_PEN));
	Rectangle(dc, entireRowRect.left(), entireRowRect.top(), entireRowRect.right(), entireRowRect.bottom());
	::SelectObject(dc, oldBrush);
	::SelectObject(dc, oldPen);

	Vect2 leftTop = row_->rect().leftTop();
	SetViewportOrgEx(dc, -leftTop.x - treeImpl_->tree()->tabSize(), -leftTop.y, 0);
	row_->drawRow(dc, treeImpl_->tree());
	row_->scanChildren(DrawRowVisitor(dc), treeImpl_->tree());
	SetViewportOrgEx(dc, 0, 0, 0);
}

BOOL DragWindow::onMessageEraseBkgnd(HDC dc)
{
	return useLayeredWindows_ ? TRUE : FALSE;
	return FALSE;
}

void DragWindow::updateLayeredWindow()
{
	BLENDFUNCTION blendFunction;
	blendFunction.BlendOp = AC_SRC_OVER;
	blendFunction.BlendFlags = 0;
	blendFunction.SourceConstantAlpha = 192;
	blendFunction.AlphaFormat = 0;	

	PAINTSTRUCT ps;
	HDC realDC = BeginPaint(handle(), &ps);
	{
		Win32::MemoryDC dc(realDC);
		drawRow(dc);

		POINT leftTop = { rect_.left() + offset_.x, rect_.top() + offset_.y };
		SIZE size = { rect_.width(), rect_.height() };
		POINT point = { 0, 0 };
		UpdateLayeredWindow(handle(), realDC, &leftTop, &size, dc, &point, 0, &blendFunction, ULW_ALPHA);
		SetLayeredWindowAttributes(handle(), 0, 64, ULW_ALPHA);
	}
	EndPaint(handle(), &ps);
}

void DragWindow::onMessagePaint()
{
	if(useLayeredWindows_)
		return;

	PAINTSTRUCT ps;
	HDC dc = BeginPaint(handle_, &ps);
	{
		Win32::MemoryDC memoryDC(dc);
		drawRow(memoryDC);
	}
	EndPaint(handle_, &ps);
}

// ---------------------------------------------------------------------------

DragController::DragController(TreeImpl* treeImpl)
: treeImpl_(treeImpl)
, captured_(false)
, dragging_(false)
, before_(false)
, row_(0)
, window_(treeImpl)
, hoveredRow_(0)
, destinationRow_(0)
{
}

void DragController::beginDrag(PropertyRow* row, POINT pt)
{
	row_ = row;
	startPoint_ = pt;
	lastPoint_ = pt;
	captured_ = true;
	dragging_ = false;
}

bool DragController::dragOn(POINT screenPoint)
{
	window_.move(screenPoint.x - lastPoint_.x, screenPoint.y - lastPoint_.y);

	bool needCapture = false;
	if(!dragging_ && (Vect2(startPoint_.x, startPoint_.y) - Vect2(screenPoint.x, screenPoint.y)).length2() >= 25)
		if(row_->canBeDragged()){
			needCapture = true;
			Rect rect = row_->rect();
            rect = Rect(rect.leftTop() - treeImpl_->offset_ + Vect2(treeImpl_->tree()->tabSize(), 0), 
                         rect.rightBottom() - treeImpl_->offset_);
            
			window_.set(treeImpl_, row_, rect);
			window_.show();
			dragging_ = true;
		}

	if(dragging_){
		POINT point = screenPoint;
		ScreenToClient(treeImpl_->handle(), &point);
		trackRow(point);
	}
	lastPoint_ = screenPoint;
	return needCapture;
}

void DragController::interrupt()
{
	captured_ = false;
	dragging_ = false;
	row_ = 0;
	window_.hide();
}

void DragController::trackRow(POINT pt)
{
	hoveredRow_ = 0;
	destinationRow_ = 0;

    Vect2 point(pt.x, pt.y);
	PropertyRow* row = treeImpl_->rowByPoint(point);
	if(!row || !row_)
		return;

	row = row->nonPulledParent();
	if(!row->parent() || row->isChildOf(row_) || row == row_)
		return;

	PropertyTree* tree = treeImpl_->tree();
	float pos = (point.y - row->rect().top()) / float(row->rect().height());
	if(row_->canBeDroppedOn(row->parent(), row, tree)){
		if(pos < 0.25f){
			destinationRow_ = row->parent();
			hoveredRow_ = row;
			before_ = true;
			return;
		}
		if(pos > 0.75f){
			destinationRow_ = row->parent();
			hoveredRow_ = row;
			before_ = false;
			return;
		}
	}
	if(row_->canBeDroppedOn(row, 0, tree))
		hoveredRow_ = destinationRow_ = row;
}

void DragController::drawUnder(HDC dc)
{
	if(dragging_ && destinationRow_ == hoveredRow_ && hoveredRow_){
		Rect rowRect = hoveredRow_->rect();
		rowRect.setLeft(rowRect.left() + treeImpl_->tree()->tabSize());
		FillRect(dc, &Win32::Rect(rowRect), GetSysColorBrush(COLOR_HIGHLIGHT));
	}
}

void DragController::drawOver(HDC dc)
{
	if(!dragging_)
		return;
	
	Rect rowRect = row_->rect();
	{
		Win32::StockSelector brush(dc, GetSysColorBrush(COLOR_BTNFACE));
		Win32::StockSelector pen(dc, GetStockObject(NULL_PEN));
		Rectangle(dc, rowRect.left() - 1, rowRect.top() - 1, rowRect.right() + 3, rowRect.bottom() + 1);
	}

	if(destinationRow_ != hoveredRow_ && hoveredRow_){
		const int tickSize = 4;
		Rect hoveredRect = hoveredRow_->rect();
		hoveredRect.setLeft(hoveredRect.left() + treeImpl_->tree()->tabSize());

		if(!before_){ // previous
			RECT rect = { hoveredRect.left() - 1 , hoveredRect.bottom() - 1, hoveredRect.right() + 1, hoveredRect.bottom() + 1 };
			RECT rectLeft = { hoveredRect.left() - 1 , hoveredRect.bottom() - tickSize, hoveredRect.left() + 1, hoveredRect.bottom() + tickSize };
			RECT rectRight = { hoveredRect.right() - 1 , hoveredRect.bottom() - tickSize, hoveredRect.right() + 1, hoveredRect.bottom() + tickSize };
			FillRect(dc, &rect, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectLeft, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectRight, GetSysColorBrush(COLOR_HIGHLIGHT));
		}
		else{ // next
			RECT rect = { hoveredRect.left() - 1 , hoveredRect.top() - 1, hoveredRect.right() + 1, hoveredRect.top() + 1 };
			RECT rectLeft = { hoveredRect.left() - 1 , hoveredRect.top() - tickSize, hoveredRect.left() + 1, hoveredRect.top() + tickSize };
			RECT rectRight = { hoveredRect.right() - 1 , hoveredRect.top() - tickSize, hoveredRect.right() + 1, hoveredRect.top() + tickSize };
			FillRect(dc, &rect, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectLeft, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectRight, GetSysColorBrush(COLOR_HIGHLIGHT));
		}
	}
}

void DragController::drop(POINT screenPoint)
{
	PropertyTreeModel* model = treeImpl_->tree()->model();
	if(row_ && hoveredRow_){
		ASSERT(destinationRow_);
		row_->dropInto(destinationRow_, destinationRow_ == hoveredRow_ ? 0 : hoveredRow_, treeImpl_->tree(), before_);
	}

	captured_ = false;
	dragging_ = false;
	row_ = 0;
	window_.hide();
	hoveredRow_ = 0;
	destinationRow_ = 0;
}


// ---------------------------------------------------------------------------


#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
TreeImpl::TreeImpl(PropertyTree* tree)
: _ContainerWindow(tree)
, tree_(tree)
, size_(Vect2::ZERO)
, offset_(Vect2::ZERO)
, hoveredRow_(0)
, capturedRow_(0)
, drag_(this)
, area_(0, 0, 0, 0)
, redrawLock_(false)
, redrawRequest_(false)
{
	WW_VERIFY(create(L"", WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL, Rect(0, 0, 40, 40), Win32::getDefaultWindowHandle()));
}
#pragma warning(pop)

TreeImpl::~TreeImpl()
{
	tree_ = 0;
}

int TreeImpl::onMessageKillFocus(HWND newFocusedWindow)
{
	RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	return Window32::onMessageKillFocus(newFocusedWindow);
}

int TreeImpl::onMessageSetFocus(HWND lastFocusedWindow)

{
    int result = 0;
	if(!creating()){
	    result = Window32::onMessageSetFocus(lastFocusedWindow);
		owner_->_setFocus();
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}
    return result;
}

void TreeImpl::onMessageMouseWheel(SHORT delta)
{
	offset_.y = offset_.y - delta;
	updateScrollBar();
	tree_->_arrangeChildren();
	RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	__super::onMessageMouseWheel(delta);
}

void TreeImpl::onMessageLButtonDblClk(int x, int y)
{
	::SetFocus(handle_);

	Vect2 point(x, y);
	PropertyRow* row = rowByPoint(point);
	if(row){
		ASSERT(row->refCount() > 0);
		if(row->widgetRect().pointInside(pointToRootSpace(point))){
			if(!row->onActivate(tree_, true))
				toggleRow(row);	
		}
		else if(!toggleRow(row))
			row->onActivate(tree_, false);
	}
	__super::onMessageLButtonDblClk(x, y);
}

void TreeImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	ASSERT(::IsWindow(handle_));
	HWND focusedWindow = GetFocus();
	if(focusedWindow){
		ASSERT(::IsWindow(focusedWindow));
		Win32::Window32 wnd(focusedWindow);
		if(Win32::Window32* window = reinterpret_cast<Win32::Window32*>(wnd.getUserDataLongPtr())){
			ASSERT(window->handle() == focusedWindow);
		}
	}
	//::SetFocus(handle_);

	if(drag_.captured()){
		POINT pos;
		GetCursorPos(&pos);
		if(GetCapture() == handle())
			ReleaseCapture();
		drag_.drop(pos);
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE);
	}
	Vect2 point(x, y);
	PropertyRow* row = rowByPoint(point);
	if(row){
		switch(hitTest(row, point, row->rect())){
		case TREE_HIT_ROW:
			if(hoveredRow_)
				RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		}			
	}
	if(capturedRow_){
		Rect rowRecti = capturedRow_->rect();
		tree_->onRowLMBUp(capturedRow_, rowRecti, pointToRootSpace(Vect2(x, y)));
		capturedRow_ = 0;
	}
	
	__super::onMessageLButtonUp(button, x, y);
}

void TreeImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	ASSERT(::IsWindow(handle_));
	::SetFocus(handle_);
	PropertyRow* row = rowByPoint(Vect2(x, y));
	if(row){
		if(tree_->onRowLMBDown(row, row->rect(), pointToRootSpace(Vect2(x, y))))
			capturedRow_ = row;
		else{
			// row могла уже быть пересоздана	
			row = rowByPoint(Vect2(x, y));
            if(row && !row->userReadOnly()){
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				drag_.beginDrag(row, cursorPos);
            }
		}
	}
	else
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}

void TreeImpl::onMessageRButtonDown(UINT button, int x, int y)
{
	ASSERT(::IsWindow(handle_));
	::SetFocus(handle_);

	Vect2 point(x, y);
	PropertyRow* row = rowByPoint(point);
	if(row){
		model()->setFocusedRow(row);
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

		tree_->onRowRMBDown(row, row->rect(), pointToRootSpace(point));
	}
	else{
		Win32::Rect rect;
		GetClientRect(handle(), &rect);
		tree_->onRowRMBDown(model()->root(), rect.recti(), pointToRootSpace(point));
	}
	__super::onMessageRButtonDown(button, x, y);
}

void TreeImpl::onMessageMButtonDown(UINT button, int x, int y)
{
	::SetFocus(handle_);

	Vect2 point(x, y);
	PropertyRow* row = rowByPoint(point);
	if(row){
		switch(hitTest(row, point, row->rect())){
		case TREE_HIT_PLUS:
			break;
		case TREE_HIT_NONE:
		default:
			model()->setFocusedRow(row);
			RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		}
	}
	__super::onMessageMButtonDown(button, x, y);
}

void TreeImpl::onMessageMouseMove(UINT button, int x, int y)
{
	if(drag_.captured() && !(button & MK_LBUTTON))
		drag_.interrupt();
	if(drag_.captured()){
		POINT pos;
		GetCursorPos(&pos);
		if(drag_.dragOn(pos) && GetCapture() != handle())
			SetCapture(handle());
        RedrawWindow(handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else{
		Vect2 point(x, y);
		PropertyRow* row = rowByPoint(pointToRootSpace(point));
		if(row){
			switch(hitTest(row, point, row->rect())){
			case TREE_HIT_ROW:
				if(row != hoveredRow_){
					hoveredRow_ = row;
					//RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				}
				break;
			case TREE_HIT_PLUS:
			case TREE_HIT_TEXT:
				if(hoveredRow_){
					hoveredRow_ = 0;
					//RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				}
				break;
			case TREE_HIT_NONE:
			default:
				break;
			}
		}
		if(capturedRow_){
			Rect rect;
			getRowRect(capturedRow_, rect);
			tree_->onRowMouseMove(capturedRow_, rect, point);
		}
		__super::onMessageMouseMove(button, x, y);
	}
}

BOOL TreeImpl::onMessageEraseBkgnd(HDC dc)
{
	return FALSE;
}

void TreeImpl::onMessageScroll(UINT message, WORD type)
{
	SetFocus(handle_);
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask  = SIF_ALL;

	HDC dc = GetDC(handle());
	ASSERT(dc);
	HGDIOBJ oldFont = ::SelectObject(dc, Win32::defaultFont());
	TEXTMETRIC textMetric;
	WW_VERIFY(GetTextMetrics(dc, &textMetric));
	::SelectObject(dc, oldFont);
	int lineScrollHeight = textMetric.tmHeight + 2;
	ReleaseDC(handle(), dc);

	::GetScrollInfo(handle_, message == WM_VSCROLL ? SB_VERT : SB_HORZ, &info);
	int oldPosition = info.nPos;
	switch(type){
	case SB_TOP:        info.nPos = info.nMin; break;
	case SB_BOTTOM:     info.nPos = info.nMax; break;
	case SB_LINEUP:     info.nPos -= lineScrollHeight; break;
	case SB_LINEDOWN:   info.nPos += lineScrollHeight; break;
	case SB_PAGEUP:     info.nPos -= info.nPage; break;
	case SB_PAGEDOWN:   info.nPos += info.nPage; break;
	case SB_THUMBTRACK: info.nPos = info.nTrackPos; break;
	default:			break; 
	}

	info.fMask = SIF_POS;

	if(message == WM_VSCROLL){
		::SetScrollInfo(handle_, SB_VERT, &info, TRUE);
		::GetScrollInfo(handle_, SB_VERT, &info);
		offset_.y = int(info.nPos);
	}
	else{
		::SetScrollInfo(handle_, SB_HORZ, &info, TRUE);
		::GetScrollInfo(handle_, SB_HORZ, &info);
		offset_.x = int(info.nPos);
	}

	if(info.nPos != oldPosition){
		tree_->_arrangeChildren();
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		updateScrollBar();
	}
}

void TreeImpl::updateArea()
{
    Win32::Rect clientRect;
    ::GetClientRect(handle_, &clientRect);

    area_ = clientRect.recti();
    area_.setLeft(area_.left() + 1);
    area_.setRight(area_.right() - 1);
    area_.setTop(area_.top() + 1);
    area_.setBottom(area_.bottom() - 1);

    if (tree_->filterMode_)
        area_.setTop(area_.top() + tree_->filterEntry_->_minimalSize().y + 2 + 2);

    tree_->_arrangeChildren();
    updateScrollBar();

    if(area_.width() > 0)
        tree_->needUpdate_ = true;
}

BOOL TreeImpl::onMessageSize(UINT type, USHORT width, USHORT height)
{
	if(!creating()){
        updateArea();
	}
	return TRUE;
}

void TreeImpl::onMessagePaint()
{
	if(redrawLock_){
		redrawRequest_ = true;
		return;
	}
	redrawRequest_ = false;

	PAINTSTRUCT ps;
	HDC dc = BeginPaint(handle_, &ps);
	{
		Win32::MemoryDC memoryDC(dc);
		redraw(memoryDC);
	}
	EndPaint(handle_, &ps);
	__super::onMessagePaint();
}

int TreeImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
    KeyPress keyPress = KeyPress::addModifiers(Key(keyCode));

	if (keyPress == KeyPress(KEY_F, KEY_MOD_CONTROL)) {
        tree_->setFilterMode(true);
    }

    if (tree_->filterMode_) {
        if (keyPress == KeyPress(KEY_ESCAPE)) {
            tree_->setFilterMode(false);
        }
    }

	PropertyRow* row = model()->focusedRow();
	bool result = tree_->onRowKeyDown(row, keyPress);
	::RedrawWindow(handle(), 0, 0, RDW_INVALIDATE);
	if(!result)
		return __super::onMessageKeyDown(keyCode, count, flags);
	else
		return 0;
}

int TreeImpl::onMessageChar(UINT code, USHORT count, USHORT flags)
{
	if (code >= 0x20 && code != VK_ESCAPE)
	{
        if (!(code == VK_BACK && !tree_->filterMode_))
            tree_->setFilterMode(true);
	   PostMessage(tree_->filterEntry_->_window()->handle(), WM_CHAR, code, MAKELPARAM(count, flags));
	}
	return __super::onMessageChar(code, count, flags);
}

int TreeImpl::onMessageGetDlgCode(int keyCode, MSG* msg)
{
	if (!msg)
		return DLGC_WANTMESSAGE;
	else {
		if (tree_->filterMode_) {
			if (keyCode == VK_ESCAPE)
				return DLGC_WANTMESSAGE;
		}
		if (keyCode == VK_RETURN)
			return DLGC_WANTMESSAGE;
	}
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

LRESULT TreeImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	redrawLock_ = true;
	ASSERT(::IsWindow(handle_));
    
	switch(message){
	case WM_HSCROLL:
	case WM_VSCROLL:
		onMessageScroll(message, LOWORD(wparam));
		break;
	case WM_ENABLE:
		ASSERT(wparam && "Disabling Window");
		break;
    }
    LRESULT result = __super::onMessage(message, wparam, lparam);
	
	redrawLock_ = false; 
	if(redrawRequest_) 
		onMessagePaint();
	
	return result;
}

struct DrawVisitor
{
	DrawVisitor(HDC dc, const Rect& area, int scrollOffset)
		: area_(area)
		, dc_(dc)
		, offset_(0)
		, scrollOffset_(scrollOffset)
	{}

	ScanResult operator()(PropertyRow* row, PropertyTree* tree)
	{
		if(row->visible(tree) && (row->parent()->expanded() || row->pulledUp())){
			if(row->rect().top() > scrollOffset_ + area_.height())
				return SCAN_FINISHED;

			if (row->rect().bottom() > scrollOffset_)
				row->drawRow(dc_, tree);

			return SCAN_CHILDREN_SIBLINGS;
		}
		else
			return SCAN_SIBLINGS;
	}

protected:
	HDC dc_;
	Rect area_;
	int offset_;
	int scrollOffset_;
};

void TreeImpl::redraw(HDC dc)
{
	if(tree_->needUpdate_)
		tree_->updateHeights();

	RECT clientRect = { area_.left(), area_.top(), area_.right(), area_.bottom() };
    ::GetClientRect(handle_, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

	{
		Graphics gr(dc);
		gr.FillRectangle(&SolidBrush(gdiplusSysColor(COLOR_BTNFACE)), clientRect.left, 
			clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	}

	if(tree_->filterMode_)
	{
		Win32::AutoSelector font(dc, Win32::defaultBoldFont());

		SetBkMode(dc, TRANSPARENT);
		const wchar_t filterStr[] = L"Filter:";
		Vect2 size = Win32::calculateTextSize(handle(), Win32::defaultBoldFont(), filterStr);
		int right = tree_->filterEntry_->_position().left();
		ExtTextOutW(dc, right - size.x - 6, 6, 0, 0, filterStr, ARRAY_LEN(filterStr), 0);
	}

	::IntersectClipRect(dc, area_.left(), area_.top(), area_.right(), area_.bottom());

	OffsetViewportOrgEx(dc, -offset_.x, -offset_.y, 0);

	if(drag_.captured())
		drag_.drawUnder(dc);

	OffsetViewportOrgEx(dc, area_.left(), area_.top(), 0);
	model()->root()->scanChildren(DrawVisitor(dc, area_, offset_.y), tree_);
	OffsetViewportOrgEx(dc, -area_.left(), -area_.top(), 0);
	OffsetViewportOrgEx(dc, offset_.x, offset_.y, 0);

	::IntersectClipRect(dc, area_.left() - 1, area_.top() - 1, area_.right() + 1, area_.bottom() + 1);

	if(size_.y > clientHeight)
	{
		using namespace Gdiplus;
		using Gdiplus::Rect;
		using Gdiplus::Color;
		Graphics gr(dc);
		const int shadowHeight = 10;
		Color color1(0, 0, 0, 0);
		Color color2(96, 0, 0, 0);

		Rect upperRect(area_.left() - 1, area_.top() - 1, area_.width() + 2, shadowHeight + 1);
		LinearGradientBrush upperBrush(upperRect, color2, color1, LinearGradientModeVertical);
		upperRect.Y += 1;
		upperRect.Height -= 2;
		gr.FillRectangle(&upperBrush, upperRect);

		Rect lowerRect(area_.left(), area_.bottom() - shadowHeight / 2 - 1, area_.width(), shadowHeight / 2 + 2);
		LinearGradientBrush lowerBrush(lowerRect, color1, color2, LinearGradientModeVertical);
		lowerRect.Y += 1;
		lowerRect.Height -= 2;
		gr.FillRectangle(&lowerBrush, lowerRect);

		SolidBrush brush(gdiplusSysColor(COLOR_BTNSHADOW));
		gr.FillRectangle(&brush, Rect(clientRect.left, area_.top(), 1, area_.height()));
		gr.FillRectangle(&brush, Rect(clientRect.left, area_.top(), area_.width(), 2));
		gr.FillRectangle(&brush, Rect(clientRect.right - 1, area_.top(), 1, area_.height()));
		gr.FillRectangle(&brush, Rect(clientRect.left, area_.bottom() - 2, area_.width(), 2));
	}

	if(drag_.captured()){
		OffsetViewportOrgEx(dc, -offset_.x, -offset_.y, 0);
		drag_.drawOver(dc);
		OffsetViewportOrgEx(dc, offset_.x, offset_.y, 0);
	}
	else{
		if(model()->focusedRow()->isRoot() && tree_->hasFocus()){
			clientRect.left += 2; clientRect.top += 2;
			clientRect.right -= 2; clientRect.bottom -= 2;
			DrawFocusRect(dc, &clientRect);
		}
	}
}

TreeImpl::TreeHitTest TreeImpl::hitTest(PropertyRow* row, Vect2 pointInWindowSpace, const Rect& rowRect)
{
	Vect2 point = pointToRootSpace(pointInWindowSpace);

	if(!row->hasVisibleChildren(tree()) && row->plusRect().pointInside(point))
		return TREE_HIT_PLUS;

	if(row->textRect().pointInside(point))
		return TREE_HIT_TEXT;
	
	if(rowRect.pointInside(point))
		return TREE_HIT_ROW;
	
	return TREE_HIT_NONE;
}

Vect2 TreeImpl::pointToRootSpace(Vect2 point) const
{
	return Vect2(point.x + offset_.x - area_.left(), point.y + offset_.y - area_.top());
}

PropertyRow* TreeImpl::rowByPoint(Vect2 point)
{
	if (!area_.pointInside(point))
		return 0;
    return model()->root()->hit(tree_, pointToRootSpace(point));
}

bool TreeImpl::toggleRow(PropertyRow* row)
{
	if(!row->canBeToggled(tree_))
		return false;
	tree_->expandRow(row, !row->expanded());

	updateScrollBar();
	::RedrawWindow(handle_, 0, 0, RDW_INVALIDATE);
	return true;
}

struct RowRectObtainer 
{
	RowRectObtainer(PropertyRow* row)
	: offset_(0)
	, row_(row)
	{}

	ScanResult operator()(PropertyRow* row, PropertyTree* tree)
	{
		if(row == row_)
			return SCAN_FINISHED;
		offset_ += row->height();
		return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
	}

	int offset_;
	PropertyRow* row_;
};

bool TreeImpl::getRowRect(PropertyRow* row, Rect& outRect, bool onlyVisible)
{
    if(!model()->root())
        return false;
	RowRectObtainer obtainer(row);
	model()->root()->scanChildren(obtainer, tree());

	int height = row->pulledUp() ? row->parent()->height() : row->height();
	int offset = row->pulledUp() ? obtainer.offset_ - height : obtainer.offset_;

	if(!onlyVisible || offset >= offset_.y && offset - offset_.y < area_.bottom()){
		int top = area_.top() + offset - offset_.y;
		outRect.set(area_.left(), top, area_.right(), top + height);
		return true;
	}
	return false;
}

void TreeImpl::ensureVisible(PropertyRow* row, bool update)
{
    ESCAPE(row != 0, return);
	if(row->isRoot())
		return;

	tree_->expandParents(row);

	Rect rect = row->rect();
	if(rect.top() < area_.top() + offset_.y){
		offset_.y =  rect.top() - area_.top();
	}
	else if(rect.bottom() > area_.bottom() + offset_.y){
		offset_.y = rect.bottom() - area_.bottom();
	}
	if(update)
		tree_->update();
}

void TreeImpl::updateScrollBar()
{
	int pageSize = area_.height();
	offset_.x = max(0, min(offset_.x, max(0, size_.x - area_.right() - 1)));
	offset_.y = max(0, min(offset_.y, max(0, size_.y - pageSize)));

	SCROLLINFO scrollInfo;

	memset((void*)&scrollInfo, 0, sizeof(scrollInfo));
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrollInfo.nMin = 0;
	scrollInfo.nMax = size_.y;
	scrollInfo.nPos = offset_.y;
	scrollInfo.nPage = pageSize;
	SetScrollInfo(handle_, SB_VERT, &scrollInfo, TRUE);

	memset((void*)&scrollInfo, 0, sizeof(scrollInfo));
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrollInfo.nMin = 0;
	scrollInfo.nMax = size_.x;
	scrollInfo.nPos = offset_.x;
	scrollInfo.nPage = area_.right() + 1;
	SetScrollInfo(handle_, SB_HORZ, &scrollInfo, TRUE);
}

}


