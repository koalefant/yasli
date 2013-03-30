/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/_WidgetWithWindow.h"
#include "ww/ConstStringList.h"
#include "yasli/Serializer.h"
#include "yasli/Object.h"
#include "ww/Strings.h"
#include <vector>

namespace Gdiplus {
	class Font;
	class Graphics;
};

namespace yasli {
	class Object;
};

namespace ww {
using std::vector;
struct Color;
class TreeImpl;
class PropertyTreeModel;
class PopupMenuItem;
class PropertyTreeModel;
class PropertyRow;
typedef std::vector<yasli::SharedPtr<PropertyRow> > PropertyRows;
class PropertyRowWidget;
class PropertyTreeOperator;
class Entry;
struct KeyPress;

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

class PropertyTree : public _ContainerWithWindow, public TreeConfig
{
public:
	PropertyTree(int border = 0);
	~PropertyTree();

	void attach(const Serializer& serializer);
	void attach(const Serializers& serializer);
	void attachPropertyTree(PropertyTree* propertyTree);
	void getSelectionSerializers(Serializers* serializers);
	void detach();
	bool attached() const { return !attached_.empty(); }

	void apply();
	void revert();

	void setCompact(bool compact) { compact_ = compact; update(); }
	void setFullRowMode(bool fullRowMode) { fullRowMode_ = fullRowMode; update(); }
	void setExpandLevels(int levels);
	void setUndoEnabled(bool enabled, bool full = false);
	void setAutoRevert(bool autoRevert) { autoRevert_ = autoRevert; }

	Vect2 treeSize() const;
	int leftBorder() const { return leftBorder_; }
	int rightBorder() const { return rightBorder_; }
	bool multiSelectable() const { return attachedPropertyTree_ != 0; }
	void onSignalChanged() { ; }

	bool spawnWidget(PropertyRow* row, bool ignoreReadOnly);
	PropertyRow* selectedRow();
	bool getSelectedObject(Object* object);
    bool selectByAddress(void*, bool keepSelectionIfChildSelected = false);
	void ensureVisible(PropertyRow* row, bool update = true);
	void expandParents(PropertyRow* row);
	void expandRow(PropertyRow* row, bool expanded = true, bool updateHeights = true);
	void expandAll(PropertyRow* root = 0);
	void collapseAll(PropertyRow* root = 0);

	void serialize(Archive& ar);

	PropertyTreeModel* model() { return model_; }
	const PropertyTreeModel* model() const { return model_; }
	void onModelUpdated(const PropertyRows& rows);

	void update(); // needs to be called after model change
	void redraw();

	bool hasFocus() const;

	// internal methods:
	void onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos);
	Vect2 _toScreen(Vect2 point) const;
	void _setFocus();
	void _cancelWidget(){ widget_ = 0; }
	void _drawRowLabel(Gdiplus::Graphics* gr, const wchar_t* text, Gdiplus::Font* font, const Rect& rect, const Color& color) const;
	void _drawRowValue(Gdiplus::Graphics* gr, const wchar_t* text, Gdiplus::Font* font, const Rect& rect, const Color& color, bool pathEllipsis, bool center) const;
	Rect _visibleRect() const;
	bool _isDragged(const PropertyRow* row) const;
	bool _isCapturedRow(const PropertyRow* row) const;
	TreeImpl* impl() const;








	signal0& signalChanged(){ return signalChanged_; }
	typedef signal1<const yasli::Object&> SignalObjectChanged;
	SignalObjectChanged& signalObjectChanged(){ return signalObjectChanged_; }
	signal0& signalSelected(){ return signalSelected_; }
	signal0& signalReverted(){ return signalReverted_; }
	signal0& signalPushUndo(){ return signalPushUndo_; }

protected:

	struct RowFilter {
		enum Type {
			NAME,
			VALUE,
			TYPE,
			NUM_TYPES
		};

		wstring start[NUM_TYPES];
		bool tillEnd[NUM_TYPES];
		vector<wstring> substrings[NUM_TYPES];

		void parse(const wchar_t* filter);
		bool match(const wchar_t* text, Type type, size_t* matchStart, size_t* matchEnd) const;
		bool typeRelevant(Type type) const{
			return !start[type].empty() || !substrings[type].empty();
		}

		RowFilter()
		{
			for (int i = 0; i < NUM_TYPES; ++i)
				tillEnd[i] = false;
		}
	};
	void interruptDrag();
	void updateHeights();

	bool onContextMenu(PropertyRow* row, PopupMenuItem& menu);
	bool onRowKeyDown(PropertyRow* row, KeyPress key);
	// points here are specified in root-row space
	bool onRowLMBDown(PropertyRow* row, const Rect& rowRect, Vect2 point);
	void onRowLMBUp(PropertyRow* row, const Rect& rowRect, Vect2 point);
	void onRowRMBDown(PropertyRow* row, const Rect& rowRect, Vect2 point);
	void onRowMouseMove(PropertyRow* row, const Rect& rowRect, Vect2 point);

	void onRowMenuUndo();
	void onRowMenuCopy(yasli::SharedPtr<PropertyRow> row);
	void onRowMenuPaste(yasli::SharedPtr<PropertyRow> row);
	void onRowMenuDecompose(PropertyRow* row);
	void onFilterChanged();

	void onModelPushUndo(PropertyTreeOperator* op, bool* handled);
	bool activateRow(PropertyRow* row);
	bool canBePasted(PropertyRow* destination);
	bool canBePasted(const char* destinationType);

	void setFilterMode(bool inFilterMode);
	void startFilter(wstring filter);
	void setWidget(PropertyRowWidget* widget);
	void _arrangeChildren();
	void visitChildren(WidgetVisitor& visitor) const;

	void updateAttachedPropertyTree();
	void drawFilteredString(Gdiplus::Graphics* gr, const wchar_t* text, RowFilter::Type type, Gdiplus::Font* font, const Rect& rect, const Color& color, bool pathEllipsis, bool center) const;

	PolyPtr<PropertyTreeModel> model_;
	int cursorX_;

	signal0 signalChanged_;
	SignalObjectChanged signalObjectChanged_;
	signal0 signalReverted_;
	signal0 signalSelected_;
    signal0 signalPushUndo_;

	PolyPtr<PropertyRowWidget> widget_; // in-place widget

	ConstStringList constStrings_;
	Serializers attached_;
	PropertyTree* attachedPropertyTree_;
	PropertyRow* capturedRow_;
	Vect2 pressPoint_;

	bool filterMode_;
	RowFilter rowFilter_;
	PolyPtr<Entry> filterEntry_; 

	bool autoRevert_;
	int leftBorder_;
	int rightBorder_;

	friend class TreeImpl;
	friend class FilterEntry;
	friend struct FilterVisitor;
};


wstring generateDigest(Serializer& ser);

}

// vim:ts=4
