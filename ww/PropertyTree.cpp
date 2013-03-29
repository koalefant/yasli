/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/PointersImpl.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyDrawContext.h"
#include "ww/Serialization.h"
#include "ww/PropertyTreeModel.h"
#include "ww/TreeImpl.h"
#include "ww/Window.h"
#include "ww/Color.h"

#include "yasli/ClassFactory.h"

#include "ww/PropertyRowContainer.h"
#include "ww/PropertyRowPointer.h"
#include "ww/Clipboard.h"
#include "ww/Unicode.h"

#include "PropertyOArchive.h"
#include "PropertyIArchive.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h" 
#include "ww/Win32/Handle.h"
#include "ww/Win32/Rectangle.h"
#include <crtdbg.h>

#include "ww/PropertyEditor.h"
#include "gdiplusUtils.h"


namespace ww{

class FilterEntry : public Entry
{
public:
	FilterEntry(PropertyTree* tree)
	: tree_(tree)
	{
		setSwallowArrows(true);
		setSwallowReturn(true);
		setSwallowEscape(true);
	}
protected:
	bool onKeyPress(const KeyPress& key)
	{
		if (key.key == KEY_UP ||
			key.key == KEY_DOWN ||
			key.key == KEY_ESCAPE ||
			key.key == KEY_RETURN)
		{
			SetFocus(tree_->impl()->handle());
			PostMessageW(tree_->impl()->handle(), WM_KEYDOWN, key.key, 0);
			return true;
		}
		if (key.key == KEY_BACK && text()[0] == '\0')
		{
			tree_->setFilterMode(false);
		}
		return false;
	}
private:
	PropertyTree* tree_;
};

// ---------------------------------------------------------------------------

TreeConfig TreeConfig::defaultConfig;

TreeConfig::TreeConfig()
: immediateUpdate_(true)
, hideUntranslated_(true)
, valueColumnWidth_(.5f)
, filter_(YASLI_DEFAULT_FILTER)
, compact_(false)
, fullRowMode_(false)
, showContainerIndices_(true)
, filterWhenType_(true)
, tabSize_(PropertyRow::ROW_DEFAULT_HEIGHT)
{
}

#pragma warning(push)
#pragma warning(disable: 4355) //  'this' : used in base member initializer list
PropertyTree::PropertyTree(int border)
: _ContainerWithWindow(new TreeImpl(this), border)
, model_(0)
, cursorX_(0)
, attachedPropertyTree_(0)
, autoRevert_(true)
, needUpdate_(true)
, capturedRow_(0)
, leftBorder_(0)
, rightBorder_(0)
, filterMode_(false)
{
	(TreeConfig&)*this = defaultConfig;

	_setMinimalSize(0, 0);

	model_ = new PropertyTreeModel();
	model_->setExpandLevels(expandLevels_);
	model_->setUndoEnabled(undoEnabled_);
	model_->setFullUndo(fullUndo_);

	model_->signalUpdated().connect(this, &PropertyTree::onModelUpdated);
	model_->signalPushUndo().connect(this, &PropertyTree::onModelPushUndo);

	filterEntry_ = new FilterEntry(this);
	filterEntry_->_setParent(this);
	filterEntry_->signalChanged().connect(this, &PropertyTree::onFilterChanged);

	DrawingCache::get()->initialize();
}
#pragma warning(pop)

PropertyTree::~PropertyTree()
{
	DrawingCache::get()->finalize();
}

void PropertyTree::update()
{
	::InvalidateRect(impl()->handle(), 0, FALSE);
}

TreeImpl* PropertyTree::impl() const
{
	return static_cast<TreeImpl*>(_window());
}

bool PropertyTree::onRowKeyDown(PropertyRow* row, KeyPress key)
{
	if(row->onKeyDown(this, key))
		return true;

    switch(key.fullCode){
	case 'C' | KEY_MOD_CONTROL:
	    onRowMenuCopy(row);
		return true;
    case 'V' | KEY_MOD_CONTROL:
	    onRowMenuPaste(row);
		return true;
    case 'Z' | KEY_MOD_CONTROL:
		if(model()->canUndo()){
            model()->undo();
			return true;
		}
		break;
	case KEY_F2:
		if(selectedRow())
			selectedRow()->onActivate(this, true);
		break;
	case KEY_APPS: {
		PopupMenu menu;
		if(onContextMenu(row, menu.root())){
            Win32::Rect rect(row->rect());
			_window()->clientToScreen(rect);
			Vect2 pt(rect.left + (rect.bottom - rect.top), rect.bottom);
			menu.spawn(pt, this);
		}
		return true; }
	}

	PropertyRow* focusedRow = model()->focusedRow();
	if(!focusedRow)
		return false;
	PropertyRow* parentRow = focusedRow->nonPulledParent();
	int x = parentRow->horizontalIndex(this, focusedRow);
	int y = model()->root()->verticalIndex(this, parentRow);
	PropertyRow* selectedRow = 0;
	switch(key.fullCode){
	case KEY_UP:
		if (filterMode_ && y == 0) {
			setFilterMode(true);
		}
		else {
			selectedRow = model()->root()->rowByVerticalIndex(this, --y);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case KEY_DOWN:
		if (filterMode_ && filterEntry_->hasFocus()) {
			setFocus();
		}
		else {
			selectedRow = model()->root()->rowByVerticalIndex(this, ++y);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case KEY_LEFT:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = --x);
		if(selectedRow == focusedRow && parentRow->canBeToggled(this) && parentRow->expanded()){
			expandRow(parentRow, false);
			//model()->requestUpdate();
			selectedRow = model()->focusedRow();
		}
		break;
	case KEY_RIGHT:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = ++x);
		if(selectedRow == focusedRow && parentRow->canBeToggled(this) && !parentRow->expanded()){
			expandRow(parentRow, true);
			//model()->requestUpdate();
			selectedRow = model()->focusedRow();
		}
		break;
	case KEY_HOME:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MIN);
		break;
	case KEY_HOME | KEY_MOD_CONTROL:
		selectedRow = model()->root()->rowByVerticalIndex(this, 0);
		if (selectedRow)
			selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		break;
	case KEY_END:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MAX);
		break;
	case KEY_END | KEY_MOD_CONTROL:
		selectedRow = model()->root()->rowByVerticalIndex(this, INT_MAX);
		if (selectedRow)
			selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		break;
	case KEY_SPACE:
		if (filterWhenType_)
			break;
	case KEY_RETURN:
		if(focusedRow->canBeToggled(this))
			expandRow(focusedRow, !focusedRow->expanded());
		else
			focusedRow->onActivate(this, false);
		break;
	}
	if(selectedRow){
		onRowSelected(selectedRow, false, false);	
		return true;
	}
	return false;
}

bool PropertyTree::onRowLMBDown(PropertyRow* row, const Rect& rowRect, Vect2 point)
{
	pressPoint_ = point;
	row = model()->root()->hit(this, point);
	if(row){
		if(!row->isRoot() && row->plusRect().pointInside(point) && impl()->toggleRow(row))
			return true;
		PropertyRow* rowToSelect = row;
		while (rowToSelect && !rowToSelect->isSelectable())
			rowToSelect = rowToSelect->parent();
		if (rowToSelect)
			onRowSelected(rowToSelect, multiSelectable() && Win32::isKeyPressed(KEY_CONTROL), true);	
	}

	PropertyTreeModel::UpdateLock lock = model()->lockUpdate();
	row = model()->root()->hit(this, point);
	if(row && !row->isRoot()){
		bool changed = false;
		bool capture = row->onMouseDown(this, point, changed);
		if(!changed && !widget_){ // FIXME: осмысленный метод для проверки
			if(capture)
				return true;
			else if(row->widgetRect().pointInside(point)){
				if(row->widgetPlacement() != PropertyRow::WIDGET_ICON)
					interruptDrag();
				row->onActivate(this, false);
				return false;
			}
		}
	}
	return false;
}

void PropertyTree::onRowLMBUp(PropertyRow* row, const Rect& rowRect, Vect2 point)
{
	row->onMouseUp(this, point);

	if(GetCapture() == _window()->handle())
		ReleaseCapture();

	if ((pressPoint_ - point).length() < 1 && row->widgetRect().pointInside(point)) {
		row->onActivateRelease(this);
	}
}

void PropertyTree::onRowRMBDown(PropertyRow* row, const Rect& rowRect, Vect2 point)
{
    SharedPtr<PropertyRow> handle = row;
	PopupMenu menu;
	PropertyRow* menuRow = 0;

	if (row->isSelectable()){
		menuRow = row;
	}
	else{
		if (row->parent() && row->parent()->isSelectable())
			menuRow = row->parent();
	}

	if (menuRow) {
		onRowSelected(menuRow, false, true);	
		if(onContextMenu(menuRow, menu.root()))
			menu.spawn(this);
	}
}

void PropertyTree::expandParents(PropertyRow* row)
{
	bool hasChanges = false;
	typedef std::vector<PropertyRow*> Parents;
	Parents parents;
	PropertyRow* p = row->nonPulledParent()->parent();
	while(p){
		parents.push_back(p);
		p = p->parent();
	}
	Parents::iterator it;
	for(it = parents.begin(); it != parents.end(); ++it) {
		PropertyRow* row = *it;
		if (!row->expanded()) {
			row->_setExpanded(true);
			hasChanges = true;
		}
	}
	if (hasChanges)
		updateHeights();
}

void PropertyTree::expandAll(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		PropertyRow::iterator it;
		for (PropertyRows::iterator it = root->begin(); it != root->end(); ++it){
			PropertyRow* row = *it;
			row->setExpandedRecursive(this, true);
		}
	}
	else
		root->setExpandedRecursive(this, true);

	for (PropertyRow* r = root; r != 0; r = r->parent())
		r->setLayoutChanged();

	updateHeights();
}

void PropertyTree::collapseAll(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		PropertyRow::iterator it;
		for (PropertyRows::iterator it = root->begin(); it != root->end(); ++it){
			PropertyRow* row = *it;
			row->setExpandedRecursive(this, false);
		}
	}
	else{
		root->setExpandedRecursive(this, false);
		PropertyRow* row = model()->focusedRow();
		while(row){
			if(root == row){
				model()->selectRow(row, true);
				break;
			}
			row = row->parent();
		}
	}

	for (PropertyRow* r = root; r != 0; r = r->parent())
		r->setLayoutChanged();

	updateHeights();
}


void PropertyTree::expandRow(PropertyRow* row, bool expanded, bool updateHeights)
{
	bool hasChanges = false;
	if (row->expanded() != expanded) {
	row->_setExpanded(expanded);
		hasChanges = true;
	}

	for (PropertyRow* r = row; r != 0; r = r->parent())
		r->setLayoutChanged();

    if(!row->expanded()){
		PropertyRow* f = model()->focusedRow();
		while(f){
			if(row == f){
				model()->selectRow(row, true);
				break;
			}
			f = f->parent();
		}
	}

	if (hasChanges && updateHeights)
		this->updateHeights();
}

void PropertyTree::interruptDrag()
{
	impl()->drag_.interrupt();
}

void PropertyTree::updateHeights()
{
	model()->root()->updateLabel(this, 0);
	int lb = compact_ ? 0 : 4;
	int rb = impl()->area_.size().x - lb*2;
	bool force = lb != leftBorder() || rb != rightBorder();
	leftBorder_ = lb;
	rightBorder_ = rb;
	model()->root()->calculateMinimalSize(this, leftBorder_, force, 0, 0);

	impl()->size_.y = 0;
	model()->root()->adjustVerticalPosition(this, impl()->size_.y);
	impl()->updateScrollBar();
	update();
}

Vect2 PropertyTree::treeSize() const
{
	return impl()->size_ + (compact() ? Vect2::ZERO : Vect2(8, 8));
}

void PropertyTree::serialize(Archive& ar)
{
	__super::serialize(ar);
	if(ar.filter(SERIALIZE_STATE)){
		bool focused = hasFocus();
		ar(focused, "focused", 0);
		ar(impl()->offset_, "offset", 0);		
		model()->serialize(ar, this);

		if(ar.isInput() && model()->root()){
			ensureVisible(model()->focusedRow());
			updateAttachedPropertyTree();
			if(focused)
				setFocus();
			update();
			signalSelected_.emit();
		}
	}
}

void PropertyTree::ensureVisible(PropertyRow* row, bool update)
{
	impl()->ensureVisible(row, update);
}

void PropertyTree::redraw()
{
	::InvalidateRect(impl()->handle(), 0, FALSE);
}

void PropertyTree::_setFocus()
{
	_ContainerWithWindow::_setFocus();
}

void PropertyTree::onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos)
{
	if(!row->isRoot())
		model()->selectRow(row, !(addSelection && row->selected() && model()->selection().size() > 1), !addSelection);
	ensureVisible(row);
	if(adjustCursorPos)
		cursorX_ = row->nonPulledParent()->horizontalIndex(this, row);
	updateAttachedPropertyTree();
	signalSelected_.emit();
}

void PropertyTree::attach(const Serializers& serializers)
{

	// We can't perform plain copying here, as it was before:
	//   attached_ = serializers;
	// ...as move forwarder calls copying constructor with non-const argument
	// which invokes second templated constructor of Serializer, which is not what we need.
	attached_.assign(serializers.begin(), serializers.end());

	revert();
}

void PropertyTree::attach(const Serializer& serializer)
{
	attached_.clear();
	attached_.push_back(serializer);
	revert();
}


void PropertyTree::detach()
{
	if(widget_)
		widget_ = 0;
	attached_.clear();
	model()->clear();
	update();
}

void PropertyTree::revertChanged(bool enforce)
{
	PropertyOArchive oa(model_, model_->root());
	oa.setFilter(filter_);

	Serializers::iterator it = attached_.begin();
	(*it)(oa);
	while(++it != attached_.end()){
		PropertyTreeModel model2;
		PropertyOArchive oa2(&model2, model_->root());
		Archive::Context<ww::PropertyTree> treeContext(oa2, this);
		oa2.setFilter(filter_);
		(*it)(oa2);
		model_->root()->intersect(model2.root());
	}

	updateHeights();
}

void PropertyTree::revert()
{
	interruptDrag();
	widget_ = 0;

	if (!attached_.empty()) {
		revertChanged(true);
	}
	else
		model_->clear();

	if (filterMode_) {
		if (model_->root())
			model_->root()->updateLabel(this, 0);
		onFilterChanged();
	}

	update();
	updateAttachedPropertyTree();
	signalReverted_.emit();
}

void PropertyTree::apply()
{
	applyChanged(true);
}

void PropertyTree::applyChanged(bool enforce)
{
	if (!attached_.empty()) {
		Serializers::iterator it;
		FOR_EACH(attached_, it) {
			PropertyIArchive ia(model_, model_->root());
 			Archive::Context<ww::PropertyTree> treeContext(ia, this);
 			ia.setFilter(filter_);
			(*it)(ia);
		}
	}

	updateHeights();
}

bool PropertyTree::spawnWidget(PropertyRow* row, bool ignoreReadOnly)
{
	if(!widget_ || widget_->row() != row){
		interruptDrag();
		setWidget(0);
		PropertyRowWidget* newWidget = 0;
		if (ignoreReadOnly && row->userReadOnlyRecurse() || !row->userReadOnly())
			newWidget = row->createWidget(this);
		setWidget(newWidget);
		return newWidget != 0;
	}
	return false;
}

bool PropertyTree::activateRow(PropertyRow* row)
{
	interruptDrag();
	return row->onActivate(this, false);
}



static wstring quoteIfNeeded(const wchar_t* str)
{
	if (wcschr(str, L' ') != 0) {
		wstring result;
		result = L"\"";
		result += str;
		result += L"\"";
		return result;
	}
	else {
		return wstring(str);
	}
}

bool PropertyTree::onContextMenu(PropertyRow* row, PopupMenuItem& menu)
{
	SharedPtr<PropertyRow> r(row);

	PropertyRow::iterator it;
	for(it = row->begin(); it != r->end(); ++it){
		PropertyRow* child = *it;
		if(child->isContainer() && child->pulledUp())
			child->onContextMenu(menu, this);
	}
	r->onContextMenu(menu, this);
	if(undoEnabled_){
		if(!menu.empty())
			menu.addSeparator();
		menu.add(TRANSLATE("Undo"))
			.connect(this, &PropertyTree::onRowMenuUndo)
			.setHotkey(KeyPress(KEY_Z, KEY_MOD_CONTROL))
			.enable(model()->canUndo());
	}
	if(!menu.empty())
		menu.addSeparator();
	menu.add(TRANSLATE("Copy"), r)
		.connect(this, &PropertyTree::onRowMenuCopy)
		.setHotkey(KeyPress(Key('C'), KEY_MOD_CONTROL));
    if(!r->userReadOnly()){
	    menu.add(TRANSLATE("Paste"), r)
		    .connect(this, &PropertyTree::onRowMenuPaste)
		    .enable(!r->userReadOnly())
		    .setHotkey(KeyPress(Key('V'), KEY_MOD_CONTROL))
		    .enable(canBePasted(r));
    }
	menu.addSeparator();
	PopupMenuItem& filter = menu.add("Filter by");
	{
		wstring nameFilter = quoteIfNeeded(toWideChar(row->labelUndecorated()).c_str());
		filter.add((wstring(L"Name:\t") + nameFilter).c_str(), nameFilter).connect(this, &PropertyTree::startFilter);

		wstring valueFilter = L"=";
		valueFilter += quoteIfNeeded(toWideChar(row->valueAsString().c_str()).c_str());
		filter.add((wstring(L"Value:\t") + valueFilter).c_str(), valueFilter).connect(this, &PropertyTree::startFilter);

		wstring typeFilter = L":";
		typeFilter += quoteIfNeeded(toWideChar(row->typeNameForFilter()).c_str());
		filter.add((wstring(L"Type:\t") + typeFilter).c_str(), typeFilter).connect(this, &PropertyTree::startFilter);
	}
#if 0
	menu.addSeparator();
	menu.add(TRANSLATE("Decompose"), r).connect(this, &PropertyTree::onRowMenuDecompose);
#endif
	return true;
}

void PropertyTree::onRowMouseMove(PropertyRow* row, const Rect& rowRect, Vect2 point)
{
	row->onMouseMove(this, point);
}

void PropertyTree::onRowMenuUndo()
{
    model()->undo();
}

void PropertyTree::onRowMenuCopy(SharedPtr<PropertyRow> row)
{
	Clipboard clipboard(this, &constStrings_, model());
    clipboard.copy(row);
}

void PropertyTree::onRowMenuPaste(SharedPtr<PropertyRow> row)
{
	if(!canBePasted(row))
		return;
	PropertyRow* parent = row->parent();

    model()->push(row);
	Clipboard clipboard(this, &constStrings_, model());
	if(clipboard.paste(row))
		model()->rowChanged(parent ? parent : model()->root());
	else
		YASLI_ASSERT(0 && "Unable to paste element!"); 
}

bool PropertyTree::canBePasted(PropertyRow* destination)
{
	if(destination->userReadOnly())
		return false;
	Clipboard clipboard(this, &constStrings_, model());
	return clipboard.paste(destination, true);
}

bool PropertyTree::canBePasted(const char* destinationType)
{
	Clipboard clipboard(this, &constStrings_, model());
	return clipboard.canBePastedOn(destinationType);
}

struct DecomposeProxy
{
	DecomposeProxy(SharedPtr<PropertyRow>& row) : row(row) {}

	void serialize(Archive& ar)
	{
		ar(row, "row", "Row");
	}

	SharedPtr<PropertyRow>& row;
};

void PropertyTree::onRowMenuDecompose(PropertyRow* row)
{
	SharedPtr<PropertyRow> clonedRow = row->clone();
	DecomposeProxy proxy(clonedRow);
	edit(Serializer(proxy), 0, IMMEDIATE_UPDATE, this);
}

void PropertyTree::onModelUpdated(const PropertyRows& rows)
{
	if (widget_)
		widget_ = 0;

	if (immediateUpdate_) {
		applyChanged(true);
			rows.front()->calculateMinimalSize(this, rows.front()->pos().x, true, 0, 0); 
		signalChanged_.emit();
		if (autoRevert_)
			revert();
	}

	setFocus();
	updateHeights();

	updateAttachedPropertyTree();
	if(!immediateUpdate_)
		signalChanged_.emit();
}

void PropertyTree::onModelPushUndo(PropertyTreeOperator* op, bool* handled)
{
	signalPushUndo_.emit();
}

void PropertyTree::setWidget(PropertyRowWidget* widget)
{
	PolyPtr<PropertyRowWidget> oldWidget = widget_;
	widget_ = 0;
	if(oldWidget){
		oldWidget->commit();
		YASLI_ASSERT(oldWidget->actualWidget());
		if(oldWidget->actualWidget())
			oldWidget->actualWidget()->hide();
		oldWidget->acquire();
		oldWidget = 0;
	}
	widget_ = widget;
	model()->dismissUpdate();
	if(widget){
		YASLI_ASSERT(widget_->actualWidget());
        if(widget_->actualWidget())
			widget_->actualWidget()->_setParent(this);
		_arrangeChildren();
	}
}


void PropertyTree::setFilterMode(bool inFilterMode)
{
    bool changed = filterMode_ != inFilterMode;
    filterMode_ = inFilterMode;
    
	if (filterMode_)
	{
        filterEntry_->show();
		filterEntry_->setFocus();
		filterEntry_->setSelection(ww::EntrySelection(0, -1));
	}
    else
        filterEntry_->hide();

    if (changed)
    {
        onFilterChanged();
        impl()->updateArea();
        needUpdate_ = true;
		::InvalidateRect(impl()->handle(), 0, FALSE);
    }
}

void PropertyTree::startFilter(wstring filter)
{
	setFilterMode(true);
	filterEntry_->setText(filter.c_str());
	onFilterChanged();
}


void PropertyTree::visitChildren(WidgetVisitor& visitor) const
{
	if(widget_)
		visitor(*(widget_->actualWidget()));
}

void PropertyTree::_arrangeChildren()
{
	if(widget_){
		PropertyRow* row = widget_->row();
		if(row->visible(this)){
			Widget* w = widget_->actualWidget();
			YASLI_ASSERT(w);
			if(w){
				Rect rect = row->widgetRect();
				rect = Rect(rect.leftTop() - impl()->offset_ + impl()->area_.leftTop(), 
							rect.rightBottom() - impl()->offset_ + impl()->area_.leftTop());
				w->_setPosition(rect);
				if(!w->isVisible()){
					w->show();
					w->setFocus();
				}
			}
			else{
				//YASLI_ASSERT(w);
			}
		}
		else{
			widget_ = 0;
		}
	}

    if (filterEntry_) {
        Vect2 size = _position().size();
        const int padding = 2;
		Rect pos(60, padding, size.x - padding - GetSystemMetrics(SM_CXVSCROLL), filterEntry_->_minimalSize().y + padding);
        filterEntry_->_setPosition(pos);
    }
}

bool PropertyTree::hasFocus() const
{
	HWND focusedWindow = GetFocus();
	return focusedWindow == impl()->handle() || IsChild(impl()->handle(), focusedWindow);
}

void PropertyTree::setExpandLevels(int levels)
{
	expandLevels_ = levels;
    model()->setExpandLevels(levels);
}

PropertyRow* PropertyTree::selectedRow()
{
    const PropertyTreeModel::Selection &sel = model()->selection();
    if(sel.empty())
        return 0;
    return model()->rowFromPath(sel.front());
}

Vect2 PropertyTree::_toScreen(Vect2 point) const
{
    POINT pt = { point.x - impl()->offset_.x + impl()->area_.left(), 
		         point.y - impl()->offset_.y + impl()->area_.top() };
    ClientToScreen(impl()->handle(), &pt);
    return Vect2(pt.x, pt.y);
}

bool PropertyTree::selectByAddress(void* addr, bool keepSelectionIfChildSelected)
{
	if (model()->root()) {
        PropertyRow* row = model()->root()->findByAddress(addr);

		bool keepSelection = false;
		if (keepSelectionIfChildSelected && row && !model()->selection().empty()) {
			keepSelection = true;
			TreeSelection::const_iterator it;
			for(it = model()->selection().begin(); it != model()->selection().end(); ++it){
				PropertyRow* selectedRow = model()->rowFromPath(*it);
				if (!selectedRow)
					continue;
				if (!selectedRow->isChildOf(row)){
					keepSelection = false;
					break;
				}
			}
		}

		if (!keepSelection) {
        TreeSelection sel;
        if(row)
            sel.push_back(model()->pathFromRow(row));
			if (model()->selection() != sel) {
        model()->setSelection(sel);
				if (row)
					ensureVisible(row);
				::InvalidateRect(impl()->handle(), 0, FALSE);
				return true;
			}
    }
	}
	return false;
}

void PropertyTree::setUndoEnabled(bool enabled, bool full)
{
	undoEnabled_ = enabled; fullUndo_ = full;
    model()->setUndoEnabled(enabled);
    model()->setFullUndo(full);
}

void PropertyTree::attachPropertyTree(PropertyTree* propertyTree) 
{ 
	if(attachedPropertyTree_)
		attachedPropertyTree_->signalChanged().disconnect(this);
	attachedPropertyTree_ = propertyTree; 
//	attachedPropertyTree_->signalChanged().connect(this, &PropertyTree::revert);
	updateAttachedPropertyTree(); 
}

void PropertyTree::getSelectionSerializers(Serializers* serializers)
{
		TreeSelection::const_iterator i;
		for(i = model()->selection().begin(); i != model()->selection().end(); ++i){
			PropertyRow* row = model()->rowFromPath(*i);
			if (!row)
				continue;

			Serializer ser = row->serializer();

			if (!ser) {
				while(row && (row->pulledUp() || row->pulledBefore())) {
					row = row->parent();
				}
				ser = row->serializer();
			}
			
			if (ser)
				serializers->push_back(ser);
		}
}

void PropertyTree::updateAttachedPropertyTree()
{
	if(attachedPropertyTree_) {
 		Serializers serializers;
 		getSelectionSerializers(&serializers);
 		attachedPropertyTree_->attach(serializers);
 	}
}

struct FilterVisitor
{
	const PropertyTree::RowFilter& filter_;

	FilterVisitor(const PropertyTree::RowFilter& filter) 
    : filter_(filter)
    {
    }

	static void markChildrenAsBelonging(PropertyRow* row, bool belongs)
	{
		int count = int(row->count());
		for (int i = 0; i < count; ++i)
		{
			PropertyRow* child = row->childByIndex(i);
			child->setBelongsToFilteredRow(belongs);

			markChildrenAsBelonging(child, belongs);
		}
	}
	
	static bool hasMatchingChildren(PropertyRow* row)
	{
		int numChildren = (int)row->count();
		for (int i = 0; i < numChildren; ++i)
		{
			PropertyRow* child = row->childByIndex(i);
			if (!child)
				continue;
			if (child->matchFilter())
				return true;
			if (hasMatchingChildren(child))
				return true;
		}
		return false;
	}

	ScanResult operator()(PropertyRow* row, PropertyTree* tree)
	{
		wstring label = toWideChar(row->labelUndecorated());
		bool matchFilter = filter_.match(label.c_str(), filter_.NAME, 0, 0);
		if (matchFilter)
			matchFilter = filter_.match(row->valueAsWString().c_str(), filter_.VALUE, 0, 0);
		if (matchFilter && filter_.typeRelevant(filter_.TYPE))
			matchFilter = filter_.match(toWideChar(row->typeNameForFilter()).c_str(), filter_.TYPE, 0, 0);						   
		
		int numChildren = int(row->count());
		if (matchFilter) {
			if (row->pulledBefore() || row->pulledUp()) {
				// treat pulled rows as part of parent
				PropertyRow* parent = row->parent();
				parent->setMatchFilter(true);
				markChildrenAsBelonging(parent, true);
				parent->setBelongsToFilteredRow(false);
			}
			else {
				markChildrenAsBelonging(row, true);
				row->setBelongsToFilteredRow(false);
				row->setLayoutChanged();
				row->setLabelChanged();
			}
		}
		else {
			bool belongs = hasMatchingChildren(row);
			row->setBelongsToFilteredRow(belongs);
			if (belongs) {
				tree->expandRow(row, true, false);
				for (int i = 0; i < numChildren; ++i) {
					PropertyRow* child = row->childByIndex(i);
					if (child->pulledUp())
						child->setBelongsToFilteredRow(true);
				}
			}
			else {
				row->_setExpanded(false);
				row->setLayoutChanged();
			}
		}

		row->setMatchFilter(matchFilter);
		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	string labelStart_;
};



void PropertyTree::RowFilter::parse(const wchar_t* filter)
{
	for (int i = 0; i < NUM_TYPES; ++i) {
		start[i].clear();
		substrings[i].clear();
		tillEnd[i] = false;
	}

	YASLI_ESCAPE(filter != 0, return);

	vector<wchar_t> filterBuf(filter, filter + wcslen(filter) + 1);
	CharLowerW(&filterBuf[0]);

	const wchar_t* str = &filterBuf[0];

	Type type = NAME;
	while (true)
	{
		bool fromStart = false;
		while (*str == L'^') {
			fromStart = true;
			++str;
		}

		const wchar_t* tokenStart = str;
		
		if (*str == L'\"')
		{
			tokenStart;
			++str;
			while(*str != L'\0' && *str != L'\"')
				++str;
		}
		else
		{
			while (*str != L'\0' && *str != L' ' && *str != L'*' && *str != L'=' && *str != L':')
					++str;
		}
		if (str != tokenStart) {
			if (*tokenStart == L'\"' && *str == L'\"') {
				start[type].assign(tokenStart + 1, str);
				tillEnd[type] = true;
				++str;
			}
			else
			{
				if (fromStart)
					start[type].assign(tokenStart, str);
				else
					substrings[type].push_back(wstring(tokenStart, str));
			}
		}
		while (*str == ' ')
			++str;

		if (*str == L'=') {
			type = VALUE;
			++str;
		}
		else if(*str == L':') {
			type = TYPE;
			++str;
		}
		else if (*str == L'\0')
			break;
	}
}

bool PropertyTree::RowFilter::match(const wchar_t* textOriginal, Type type, size_t* matchStart, size_t* matchEnd) const
{
	YASLI_ESCAPE(textOriginal, return false);

	wchar_t* text;
	{
		size_t textLen = wcslen(textOriginal);
		text = (wchar_t*)_malloca((textLen + 1) * sizeof(wchar_t));
		memcpy(text, textOriginal, (textLen + 1) * sizeof(wchar_t));
		CharLowerW(text);
	}
	
	const wstring &start = this->start[type];
	if (tillEnd[type]){
		if (start == text) {
			if (matchStart)
				*matchStart = 0;
			if (matchEnd)
				*matchEnd = start.size();
			return true;
		}
		else
			return false;
	}

	const vector<wstring> &substrings = this->substrings[type];

	const wchar_t* startPos = text;

	if (matchStart)
		*matchStart = 0;
	if (matchEnd)
		*matchEnd = 0;
	if (!start.empty()) {
		if (wcsncmp(text, start.c_str(), start.size()) != 0){
			_freea(text);
			return false;
		}
		if (matchEnd)
			*matchEnd = start.size();
		startPos += start.size();
	}

	size_t numSubstrings = substrings.size();
	for (size_t i = 0; i < numSubstrings; ++i) {
		const wchar_t* substr = wcsstr(startPos, substrings[i].c_str());
		if (!substr){
			_freea(text);
			return false;
		}
		startPos += substrings[i].size();
		if (matchStart && i == 0 && start.empty()) {
			*matchStart = substr - text;
		}
		if (matchEnd)
			*matchEnd = substr - text + substrings[i].size();
	}
	_freea(text);
	return true;
}

void PropertyTree::onFilterChanged()
{
	const wchar_t* filterStr = filterMode_ ? filterEntry_->textW() : L"";
	rowFilter_.parse(filterStr);
	FilterVisitor visitor(rowFilter_);
	model()->root()->scanChildrenBottomUp(visitor, this);
	updateHeights();
}

void PropertyTree::drawFilteredString(Gdiplus::Graphics* gr, const wchar_t* text, RowFilter::Type type, Gdiplus::Font* font, const Rect& rect, const Color& textColor, bool pathEllipsis, bool center) const
{
	int textLen = (int)wcslen(text);

	Gdiplus::StringFormat format;
	format.SetAlignment(center ? Gdiplus::StringAlignmentCenter : Gdiplus::StringAlignmentNear);
	format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	format.SetTrimming(pathEllipsis ? Gdiplus::StringTrimmingEllipsisPath : Gdiplus::StringTrimmingEllipsisCharacter);
	format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

	Gdiplus::RectF textRect(gdiplusRectF(rect));
	if (filterMode_) {
		size_t hiStart = 0;
		size_t hiEnd = 0;
		if (rowFilter_.match(text, type, &hiStart, &hiEnd)) {
			Gdiplus::RectF boxFull;
			Gdiplus::RectF boxStart;
			Gdiplus::RectF boxEnd;
			
			gr->MeasureString(text, textLen, font, textRect, &format, &boxFull, 0, 0);
			
			if (hiStart > 0)
				gr->MeasureString(text, (int)hiStart, font, textRect, &format, &boxStart, 0, 0);
			else {
				gr->MeasureString(text, textLen, font, textRect, &format, &boxStart, 0, 0);
				boxStart.Width = 0.0;
			}
			gr->MeasureString(text, (int)hiEnd, font, textRect, &format, &boxEnd, 0, 0);

			ww::Color highlightColor, highlightBorderColor;
			{
				highlightColor.setGDI(GetSysColor(COLOR_HIGHLIGHT));
				float h, s, v;
				highlightColor.toHSV(h, s, v);
				h -= 175.0f;
				if (h < 0.0f)
					h += 360.0f;
				highlightColor.setHSV(h, min(1.0f, s * 1.33f), 1.0f, 255);
				highlightBorderColor.setHSV(h, s * 0.5f, 1.0f, 255);
			}

			Gdiplus::SolidBrush br(Gdiplus::Color(highlightColor.argb()));
			int left = int(boxFull.X + boxStart.Width) - 1;
			int top = int(boxFull.Y);
			int right = int(boxFull.X + boxEnd.Width);
			int bottom = int(boxFull.Y + boxEnd.Height);
			Gdiplus::Rect highlightRect(left, top, right - left, bottom - top);

			fillRoundRectangle(gr, &br, highlightRect, Gdiplus::Color(highlightBorderColor.argb()) /*Gdiplus::Color(255, 255, 128)*/, 1);
		}
	}

	Gdiplus::SolidBrush brush(textColor.argb());
	gr->DrawString(text, textLen, font, textRect, &format, &brush); 
}

void PropertyTree::_drawRowLabel(Gdiplus::Graphics* gr, const wchar_t* text, Gdiplus::Font* font, const Rect& rect, const Color& textColor) const
{
	drawFilteredString(gr, text, RowFilter::NAME, font, rect, textColor, false, false);
}

void PropertyTree::_drawRowValue(Gdiplus::Graphics* gr, const wchar_t* text, Gdiplus::Font* font, const Rect& rect, const Color& textColor, bool pathEllipsis, bool center) const
{
	drawFilteredString(gr, text, RowFilter::VALUE, font, rect, textColor, pathEllipsis, center);
}

bool PropertyTree::_isDragged(const PropertyRow* row) const
{
	if (!impl()->drag_.isDragging())
		return false;
	if (impl()->drag_.draggedRow() == row)
		return true;
	return false;
}

bool PropertyTree::_isCapturedRow(const PropertyRow* row) const
{
	return capturedRow_ == row;
}
}
// vim:ts=4 sw=4:
