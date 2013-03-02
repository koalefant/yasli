/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ConstStringList.h"
#include <string>
#include "yasli/Serializer.h"
#include "yasli/Object.h"
#include "yasli/Pointers.h"
#include "PropertyRow.h"
#include <QtGui/QWidget>
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

namespace yasli { class Object; }

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
	bool showContainerIndices() const{ return showContainerIndices_; }

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

	int expandLevels_;
	bool undoEnabled_;
	bool fullUndo_;
};

struct PropertyRowMenuHandler;

class QPropertyTree : public QWidget, public TreeConfig
{
	Q_OBJECT
public:
	QPropertyTree(QWidget* parent);
	~QPropertyTree();

	void attach(const yasli::Object& object);
	void attach(const yasli::Serializer& serializer);
	void attach(const yasli::Serializers& serializers);
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

	QPoint treeSize() const;
	bool multiSelectable() const { return attachedPropertyTree_ != 0; }
	
	void onSignalChanged() { signalChanged(); }

	bool spawnWidget(PropertyRow* row, bool ignoreReadOnly);
	PropertyRow* selectedRow();
	bool getSelectedObject(yasli::Object* object);
	bool selectByAddress(void*, bool keepSelectionIfChildSelected = false);
	void ensureVisible(PropertyRow* row, bool update = true);
	void expandParents(PropertyRow* row);
	void expandRow(PropertyRow* row, bool expanded = true);

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
	bool hasFocusOrInplaceHasFocus() const;
	void addMenuHandler(PropertyRowMenuHandler* handler);
	void startFilter(const char* filter);
	ConstStringList* constStrings() { return &constStrings_; }
signals:
	void signalChanged();
	void signalObjectChanged(const yasli::Object& obj);
	void signalSelected();
	void signalReverted();
	void signalPushUndo();
public slots:
	void expandAll(PropertyRow* root = 0);
	void collapseAll(PropertyRow* root = 0);
    void onFilterChanged(const QString& str);
protected slots:
	void onScroll(int pos);
	void onModelUpdated(const PropertyRows& rows);
	void onModelPushUndo(PropertyTreeOperator* op, bool* handled);

private:
	QPropertyTree(const QPropertyTree&);
	QPropertyTree& operator=(const QPropertyTree&);
protected:
	enum HitTest{
		TREE_HIT_PLUS,
		TREE_HIT_TEXT,
		TREE_HIT_ROW,
		TREE_HIT_NONE
	};
	void applyChanged(bool enforceAll);
	void revertChanged(bool enforceAll);
	PropertyRow* rowByPoint(const QPoint& point);
	HitTest hitTest(PropertyRow* row, const QPoint& pointInWindowSpace, const QRect& rowRect);

	QSize sizeHint() const override;
	void paintEvent(QPaintEvent* ev) override;
	void moveEvent(QMoveEvent* ev) override;
	void resizeEvent(QResizeEvent* ev) override;
	void mousePressEvent(QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void mouseDoubleClickEvent(QMouseEvent* ev) override;
	void wheelEvent(QWheelEvent* ev) override;
	void keyPressEvent(QKeyEvent* ev) override;

	void updateArea();
	bool toggleRow(PropertyRow* row);

	struct RowFilter {
		enum Type {
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
	bool onRowLMBDown(PropertyRow* row, const QRect& rowRect, QPoint point);
	void onRowLMBUp(PropertyRow* row, const QRect& rowRect, QPoint point);
	void onRowRMBDown(PropertyRow* row, const QRect& rowRect, QPoint point);
	void onRowMouseMove(PropertyRow* row, const QRect& rowRect, QPoint point);


	bool activateRow(PropertyRow* row);
	bool canBePasted(PropertyRow* destination);
	bool canBePasted(const char* destinationType);

	void setFilterMode(bool inFilterMode);
	void setWidget(PropertyRowWidget* widget);
	void _arrangeChildren();

	void updateAttachedPropertyTree();
	void drawFilteredString(QPainter& p, const wchar_t* text, RowFilter::Type type, const QFont* font, const QRect& rect, const QColor& color, bool pathEllipsis, bool center) const;

	QScopedPointer<PropertyTreeModel> model_;
	int cursorX_;

	QScopedPointer<PropertyRowWidget> widget_; // in-place widget
	vector<PropertyRowMenuHandler*> menuHandlers_;

	ConstStringList constStrings_;
	vector<yasli::Object> attached_;
	QPropertyTree* attachedPropertyTree_;

	bool filterMode_;
	RowFilter rowFilter_;
	QScopedPointer<QLineEdit> filterEntry_; 

	bool autoRevert_;
	bool needUpdate_;

	QScrollBar* scrollBar_;
	QRect area_;
	QPoint size_;
	QPoint offset_;
	QSize sizeHint_;

	friend class TreeImpl;
	friend class FilterEntry;
	friend struct FilterVisitor;
	friend struct PropertyTreeMenuHandler;
};

yasli::wstring generateDigest(yasli::Serializer& ser);
