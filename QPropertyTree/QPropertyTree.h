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
#include "yasli/Serializer.h"
#include "yasli/Object.h"
#include "yasli/Pointers.h"
#include "PropertyRow.h"
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <vector>

class QMenu;
class QLineEdit;
class QScrollBar;

struct Color;
class TreeImpl;
class PropertyTreeModel;
class PopupMenuItem;
class PropertyTreeModel;
class PropertyRow;
class PropertyRowWidget;
class PropertyTreeOperator;
class Entry;
struct IconXPMCache;

namespace yasli { class Object; struct Context; }

class TreeConfig
{
public:
	TreeConfig();
	void setCompact(bool compact) { compact_ = compact; }
	bool compact() const{ return compact_; }
	void setTabSize(int tabSize) { tabSize_ = tabSize; }
	int tabSize() const { return tabSize_; }
	void setFullRowMode(bool fullRowMode) { fullRowMode_ = fullRowMode; }
	bool fullRowMode() const { return fullRowMode_; }
	void setHideUntranslated(bool hideUntranslated) { hideUntranslated_ = hideUntranslated; }
	bool hideUntranslated() const{ return hideUntranslated_; }
	void setValueColumnWidth(float valueColumnWidth) { valueColumnWidth_ = valueColumnWidth; }
	float valueColumnWidth() const { return valueColumnWidth_; }
	void setImmediateUpdate(bool immediateUpdate) { immediateUpdate_ = immediateUpdate; }
	bool immediateUpdate() const{ return immediateUpdate_; }
	void setFilter(int filter) { filter_ = filter; }
	void setFilterWhenType(bool filterWhenType) {	filterWhenType_ = filterWhenType; }
	void setExpandLevels(int levels) { expandLevels_ = levels; }
	void setUndoEnabled(bool enabled, bool full = false) { undoEnabled_ = enabled; fullUndo_ = full; }
	void setShowContainerIndices(bool showContainerIndices) { showContainerIndices_ = showContainerIndices; }
	void setSliderUpdateDelay(int delayMS) { sliderUpdateDelay_ = delayMS; }
	bool showContainerIndices() const{ return showContainerIndices_; }
	int _defaultRowHeight() const { return defaultRowHeight_; }

	static TreeConfig defaultConfig;

protected:
	bool compact_;
	bool fullRowMode_;
	bool immediateUpdate_;
	bool hideUntranslated_;
	bool showContainerIndices_;
	bool filterWhenType_;
	float valueColumnWidth_;
	int filter_;
	int tabSize_;
	int sliderUpdateDelay_;
	int defaultRowHeight_;

	int expandLevels_;
	bool undoEnabled_;
	bool fullUndo_;
};

struct PropertyRowMenuHandler;

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

class QPropertyTree : public QWidget, public TreeConfig
{
	Q_OBJECT
public:
	QPropertyTree(QWidget* parent);
	~QPropertyTree();

	void attach(const yasli::Object& object);
	void attach(const yasli::Serializer& serializer);
	bool attach(const yasli::Serializers& serializers);
	void attachPropertyTree(QPropertyTree* propertyTree);
	void getSelectionSerializers(yasli::Serializers* serializers);
	void detach();
	bool attached() const { return !attached_.empty(); }

	void apply();
	void revert();
	int revertObjects(vector<void*> objectAddresses);
	bool revertObject(void* objectAddress);


	void setCompact(bool compact) { compact_ = compact; update(); }
	void setFullRowMode(bool fullRowMode) { fullRowMode_ = fullRowMode; update(); }
	void setExpandLevels(int levels);
	void setUndoEnabled(bool enabled, bool full = false);
	void setAutoRevert(bool autoRevert) { autoRevert_ = autoRevert; }
	void setSizeHint(const QSize& size) { sizeHint_ = size; }
	void setArchiveContext(yasli::Context* lastContext);

	QPoint treeSize() const;
	const QFont& boldFont() const; 
	int leftBorder() const { return leftBorder_; }
	int rightBorder() const { return rightBorder_; }
	bool multiSelectable() const { return attachedPropertyTree_ != 0; }
	
	void onSignalChanged() { signalChanged(); }

	bool spawnWidget(PropertyRow* row, bool ignoreReadOnly);
	PropertyRow* selectedRow();
	bool getSelectedObject(yasli::Object* object);
	bool selectByAddress(const void*, bool keepSelectionIfChildSelected = false);
	bool selectByAddresses(const vector<const void*>& addresses, bool keepSelectionIfChildSelected);
	void ensureVisible(PropertyRow* row, bool update = true);
	void expandParents(PropertyRow* row);
	void expandRow(PropertyRow* row, bool expanded = true, bool updateHeights = true);
public slots:
	void expandAll(PropertyRow* root = 0);
	void collapseAll(PropertyRow* root = 0);
	void onAttachedTreeChanged();
public:

	void serialize(yasli::Archive& ar);

	PropertyTreeModel* model() { return model_.data(); }
	const PropertyTreeModel* model() const { return model_.data(); }

	// internal methods:
	void onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos);
	QPoint _toScreen(QPoint point) const;
	void _cancelWidget(){ widget_.reset(); }
	void _drawRowLabel(QPainter& p, const wchar_t* text, const QFont* font, const QRect& rect, const QColor& color) const;
	void _drawRowValue(QPainter& p, const wchar_t* text, const QFont* font, const QRect& rect, const QColor& color, bool pathEllipsis, bool center) const;
	QRect _visibleRect() const;
	bool _isDragged(const PropertyRow* row) const;
	bool _isCapturedRow(const PropertyRow* row) const;
	PropertyRow* _pressedRow() const { return pressedRow_; }
	void _setPressedRow(PropertyRow* row) { pressedRow_ = row; }
	int _applyTime() const{ return applyTime_; }
	int _revertTime() const{ return revertTime_; }
	int _updateHeightsTime() const{ return updateHeightsTime_; }
	int _paintTime() const{ return paintTime_; }
	bool hasFocusOrInplaceHasFocus() const;
	void addMenuHandler(PropertyRowMenuHandler* handler);
	IconXPMCache* _iconCache() const{ return iconCache_.data(); }

signals:
	void signalAboutToSerialize(yasli::Archive& ar);
	void signalChanged();
	void signalObjectChanged(const yasli::Object& obj);
	void signalSelected();
	void signalReverted();
	void signalPushUndo();
public slots:
    void onFilterChanged(const QString& str);
protected slots:
	void onScroll(int pos);
	void onModelUpdated(const PropertyRows& rows, bool needApply);
	void onModelPushUndo(PropertyTreeOperator* op, bool* handled);
	void onMouseStill();

private:
	QPropertyTree(const QPropertyTree&);
	QPropertyTree& operator=(const QPropertyTree&);
protected:
	class DragController;
	enum HitTest{
		TREE_HIT_PLUS,
		TREE_HIT_TEXT,
		TREE_HIT_ROW,
		TREE_HIT_NONE
	};
	PropertyRow* rowByPoint(const QPoint& point);
	HitTest hitTest(PropertyRow* row, const QPoint& pointInWindowSpace, const QRect& rowRect);
	void onRowMenuDecompose(PropertyRow* row);

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

	void updateArea();
	bool toggleRow(PropertyRow* row);

	struct RowFilter {
		enum Type {
			NAME_VALUE,
			NAME,
			VALUE,
			TYPE,
			NUM_TYPES
		};

		yasli::string start[NUM_TYPES];
		bool tillEnd[NUM_TYPES];
		std::vector<yasli::string> substrings[NUM_TYPES];

		void parse(const char* filter);
		bool match(const char* text, Type type, size_t* matchStart, size_t* matchEnd) const;
		bool typeRelevant(Type type) const{
			return !start[type].empty() || !substrings[type].empty();
		}

		RowFilter()
		{
			for (int i = 0; i < NUM_TYPES; ++i)
				tillEnd[i] = false;
		}
	};

	QPoint pointToRootSpace(const QPoint& pointInWindowSpace) const;
	void interruptDrag();
	void updateHeights();
	bool updateScrollBar();

	bool onContextMenu(PropertyRow* row, QMenu& menu);
	void clearMenuHandlers();
	bool onRowKeyDown(PropertyRow* row, const QKeyEvent* ev);
	// points here are specified in root-row space
	bool onRowLMBDown(PropertyRow* row, const QRect& rowRect, QPoint point, bool controlPressed);
	void onRowLMBUp(PropertyRow* row, const QRect& rowRect, QPoint point);
	void onRowRMBDown(PropertyRow* row, const QRect& rowRect, QPoint point);
	void onRowMouseMove(PropertyRow* row, const QRect& rowRect, QPoint point);


	bool activateRow(PropertyRow* row);
	bool canBePasted(PropertyRow* destination);
	bool canBePasted(const char* destinationType);

	void setFilterMode(bool inFilterMode);
	void startFilter(const char* filter);
	void setWidget(PropertyRowWidget* widget);
	void _arrangeChildren();

	void updateAttachedPropertyTree(bool revert);
	void drawFilteredString(QPainter& p, const wchar_t* text, RowFilter::Type type, const QFont* font, const QRect& rect, const QColor& color, bool pathEllipsis, bool center) const;

	QScopedPointer<PropertyTreeModel> model_;
	int cursorX_;

	QScopedPointer<PropertyRowWidget> widget_; // in-place widget
	vector<PropertyRowMenuHandler*> menuHandlers_;

	typedef vector<yasli::Object> Objects;
	Objects attached_;
	QPropertyTree* attachedPropertyTree_;

	bool filterMode_;
	RowFilter rowFilter_;
	QScopedPointer<QLineEdit> filterEntry_; 
	QScopedPointer<IconXPMCache> iconCache_; 
	
	bool autoRevert_;
	bool needUpdate_;

	QScrollBar* scrollBar_;
	QFont boldFont_;
	QRect area_;
	int leftBorder_;
	int rightBorder_;
	QPoint size_;
	QPoint offset_;
	QSize sizeHint_;
	DragController* dragController_;
	QPoint pressPoint_;
	QPoint lastStillPosition_;
	PropertyRow* capturedRow_;
	PropertyRow* pressedRow_;
	QTimer* mouseStillTimer_;
	yasli::Context* archiveContext_;

	int applyTime_;
	int revertTime_;
	int updateHeightsTime_;
	int paintTime_;
	bool dragCheckMode_;
	bool dragCheckValue_;

	friend class TreeImpl;
	friend class FilterEntry;
	friend class DragWindow;
	friend struct FilterVisitor;
	friend struct PropertyTreeMenuHandler;
	friend class ContainerMenuHandler;
};

yasli::wstring generateDigest(yasli::Serializer& ser);
