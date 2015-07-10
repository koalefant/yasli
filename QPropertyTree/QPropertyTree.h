/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <string>
#include "yasli/Config.h"
#include "yasli/Serializer.h"
#include "yasli/Object.h"
#include "yasli/Pointers.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/PropertyRow.h"
#include <QWidget>
#include <QPainter>
#include <vector>

class QLineEdit;
class QScrollBar;
class QPropertyTree;

namespace property_tree { 
class QDrawContext; 
class QUIFacade;
class IMenu;
}

class DragWindow : public QWidget
{
	Q_OBJECT
public:
	DragWindow(QPropertyTree* tree);

	void set(QPropertyTree* tree, PropertyRow* row, const QRect& rowRect);
	void setWindowPos(bool visible);
	void show();
	void move(int deltaX, int deltaY);
	void hide();

	void drawRow(QPainter& p);
	void paintEvent(QPaintEvent* ev);

protected:
	bool useLayeredWindows_;
	PropertyRow* row_;
	QRect rect_;
	QPropertyTree* tree_;
	QPoint offset_;
};

class PROPERTY_TREE_API QPropertyTree : public QWidget, public PropertyTree
{
	Q_OBJECT
public:
	explicit QPropertyTree(QWidget* parent = 0);
	~QPropertyTree();

	const QFont& boldFont() const; 

	// Limit number of mouse-movement updates per-frame. Used to prevent large tree updates
	// from draining all the idle time.
	void setAggregateMouseEvents(bool aggregate) { aggregateMouseEvents_ = aggregate; }
	void flushAggregatedMouseEvents();

	// Default size.
	void setSizeHint(const QSize& size) { sizeHint_ = size; }
	// Sets minimal size of the widget to the size of the visible content of the tree.
	void setSizeToContent(bool sizeToContent);
	bool sizeToContent() const{ return sizeToContent_; }
	// Retrieves size of the content, doesn't require sizeToContent to be set.
	QSize contentSize() const{ return contentSize_; }

	void attachPropertyTree(PropertyTree* propertyTree) override;

public slots:
	void onAttachedTreeChanged();
public:
	// internal methods:
	QPoint _toScreen(Point point) const;
	void _drawRowValue(QPainter& p, const char* text, const QFont* font, const QRect& rect, const QColor& color, bool pathEllipsis, bool center) const;
	int _updateHeightsTime() const{ return updateHeightsTime_; }
	int _paintTime() const{ return paintTime_; }
	bool hasFocusOrInplaceHasFocus() const override;
	bool _isDragged(const PropertyRow* row) const override;
	void _cancelWidget() override;

signals:
	// Emited for every finished changed of the value.  E.g. when you drag a slider,
	// signalChanged will be invoked when you release a mouse button.
	void signalChanged();
	// Used fast-pace changes, like movement of the slider before mouse gets released.
	void signalContinuousChange();
	// Called before and after serialization is invoked. Can be used to update context list
	// in archive.
	void signalAboutToSerialize(yasli::Archive& ar);
	void signalSerialized(yasli::Archive& ar);
	// OBSOLETE: do not use
	void signalObjectChanged(const yasli::Object& obj);
	// Invoked whenever selection changed.
	void signalSelected();
	// Invoked after each revert() call (can be caused by user intput).
	void signalReverted();
	// Invoked before any change is going to occur and can be used to store current version
	// of data for own undo stack.
	void signalPushUndo();
	// Called when visual size of the tree changes, i.e. when things are deserialized and
	// and when rows are expanded/collapsed.
	void signalSizeChanged();
public slots:
    void onFilterChanged(const QString& str);
protected slots:
	void onScroll(int pos);
	void onMouseStillTimer();

protected:
	void onAboutToSerialize(yasli::Archive& ar) override { signalAboutToSerialize(ar); }
	void onSerialized(yasli::Archive& ar) override { signalSerialized(ar); }
	void onChanged() override { signalChanged(); }
	void onContinuousChange() override { signalContinuousChange(); }
	void onSelected() override { signalSelected(); }
	void onReverted() override { signalReverted(); }
	void onPushUndo() override { signalPushUndo(); }

	void copyRow(PropertyRow* row) override;
	void pasteRow(PropertyRow* row) override;
	bool canBePasted(PropertyRow* destination) override;
	bool canBePasted(const char* destinationType) override;

	void interruptDrag() override;
	void defocusInplaceEditor() override;
	class DragController;

	void updateHeights(bool recalculateText = false) override;
	void repaint() override { update(); }
	void resetFilter() override { onFilterChanged(QString()); }

	QSize sizeHint() const override;
	void paintEvent(QPaintEvent* ev) override;
	void moveEvent(QMoveEvent* ev) override;
	void resizeEvent(QResizeEvent* ev) override;
	void mousePressEvent(QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void mouseDoubleClickEvent(QMouseEvent* ev) override;
	void mouseMoveEvent(QMouseEvent* ev) override;
	void wheelEvent(QWheelEvent* ev) override;
	void keyPressEvent(QKeyEvent* ev) override;
	void focusInEvent(QFocusEvent* ev) override;

	bool updateScrollBar() override;

	void setFilterMode(bool inFilterMode);
	void startFilter(const char* filter) override;

	using PropertyTree::pointToRootSpace;
	using PropertyTree::pointFromRootSpace;
	QPoint pointToRootSpace(const QPoint& pointInWindowSpace) const;
	QPoint pointFromRootSpace(const QPoint& point) const;

	void _arrangeChildren() override;

	void drawFilteredString(QPainter& p, const char* text, RowFilter::Type type, const QFont* font, const QRect& rect, const QColor& color, bool pathEllipsis, bool center) const;

	QScopedPointer<QLineEdit> filterEntry_; 
	
	QScrollBar* scrollBar_;
	QFont boldFont_;
	QSize sizeHint_;
	DragController* dragController_;
	QTimer* mouseStillTimer_;

	bool aggregateMouseEvents_;
	int aggregatedMouseEventCount_;
	QScopedPointer<QMouseEvent> lastMouseMoveEvent_;

	int updateHeightsTime_;
	int paintTime_;
	bool sizeToContent_;
	QSize contentSize_;

	friend class property_tree::QDrawContext;
	friend class property_tree::QUIFacade;
	friend struct PropertyTreeMenuHandler;
	friend class FilterEntry;
	friend class DragWindow;
};

yasli::wstring generateDigest(yasli::Serializer& ser);
