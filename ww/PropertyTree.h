#pragma once

#include "ww/_WidgetWithWindow.h"
#include "ww/ConstStringList.h"
#include "yasli/Serializer.h"

namespace ww{

class TreeImpl;
class PropertyTreeModel;
class PopupMenuItem;
class PropertyTreeModel;
class PropertyRow;
class PropertyRowWidget;
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
	void setExpandLevels(int levels) { expandLevels_ = levels; }
	void setUndoEnabled(bool enabled, bool full = false) { undoEnabled_ = enabled; fullUndo_ = full; }

	static TreeConfig defaultConfig;

protected:
	bool compact_;
	bool fullRowMode_;
	bool immediateUpdate_;
	bool hideUntranslated_;
	float valueColumnWidth_;
	int filter_;
	int tabSize_;

	int expandLevels_;
	bool undoEnabled_;
	bool fullUndo_;
};

class WW_API PropertyTree : public _ContainerWithWindow, public TreeConfig
{
public:
    PropertyTree(int border = 0);
    ~PropertyTree();

    void attach(Serializer serializer);
    void attach(Serializers& serializers);
	void attachPropertyTree(PropertyTree* propertyTree);
    void detach();

    void revert();
    void apply();

    void setCompact(bool compact) { compact_ = compact; update(); }
    void setFullRowMode(bool fullRowMode) { fullRowMode_ = fullRowMode; update(); }
    void setExpandLevels(int levels);
    void setUndoEnabled(bool enabled, bool full = false);
    void setAutoRevert(bool autoRevert) { autoRevert_ = autoRevert; }
	
	Vect2i treeSize() const;
	bool multiSelectable() const { return propertyTree_ != 0; }

    sigslot::signal0& signalChanged(){ return signalChanged_; }
    sigslot::signal0& signalSelected(){ return signalSelected_; }

    bool spawnWidget(PropertyRow* row, bool ignoreReadOnly);
    PropertyRow* selectedRow();
    void selectByAddress(void*);
    void ensureVisible(PropertyRow* row, bool update = true);
    void expandParents(PropertyRow* row);
    void expandRow(PropertyRow* row, bool expanded = true);
	void expandAll(PropertyRow* root = 0);
	void collapseAll(PropertyRow* root = 0);

    void serialize(Archive& ar);

    PropertyTreeModel* model() { return model_; }
    const PropertyTreeModel* model() const { return model_; }

    void update(); // нужно вызывать после изменения модели
    void redraw();

    bool _focusable() const;
    TreeImpl* impl() const;

    bool isFocused() const;
    Vect2i _toScreen(Vect2i point) const;
    void _setFocus();
    void _cancelWidget(){ widget_ = 0; }

	void onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos);

protected:
    void interruptDrag();
    void updateHeights();

    bool onContextMenu(PropertyRow* row, PopupMenuItem& menu);
    bool onRowKeyDown(PropertyRow* row, KeyPress key);
    bool onRowLMBDown(PropertyRow* row, const Rect& rowRect, Vect2i point);
    void onRowLMBUp(PropertyRow* row, const Rect& rowRect, Vect2i point);
    void onRowRMBDown(PropertyRow* row, const Rect& rowRect, Vect2i point);
    void onRowMouseMove(PropertyRow* row, const Rect& rowRect, Vect2i point);

    void onRowMenuUndo();
    void onRowMenuCopy(PropertyRow* row);
    void onRowMenuPaste(PropertyRow* row);
    void onRowMenuDecompose(PropertyRow* row);

    void onModelUpdated();
    bool activateRow(PropertyRow* row);
    bool canBePasted(PropertyRow* destination);
    bool canBePasted(const char* destinationType);

    void setWidget(PropertyRowWidget* widget);
    void _arrangeChildren();
    void visitChildren(WidgetVisitor& visitor) const;

	void updatePropertyTree();

	yasli::SharedPtr<PropertyTreeModel> model_;
	int cursorX_;

    sigslot::signal0 signalChanged_;
    sigslot::signal0 signalSelected_;

    PolyPtr<PropertyRowWidget> widget_; // in-place widget

    ConstStringList constStrings_;
    Serializers attached_;
	PropertyTree* propertyTree_;
    bool autoRevert_;
	bool needUpdate_;

    friend class TreeImpl;
};


std::wstring WW_API generateDigest(Serializer& ser);

}

