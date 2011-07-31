#include "StdAfx.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyDrawContext.h"
#include "ww/Serialization.h"
#include "ww/PropertyTreeModel.h"
#include "ww/TreeImpl.h"
#include "ww/Window.h"
#include "ww/Color.h"

#include "yasli/TypesFactory.h"

#include "ww/_PropertyRowBuiltin.h"
#include "ww/Clipboard.h"
#include "ww/Unicode.h"

#include "PropertyOArchive.h"
#include "PropertyIArchive.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window.h" 
#include "ww/Win32/Handle.h"
#include "ww/Win32/Rectangle.h"
#include <crtdbg.h>

#include "ww/PropertyEditor.h"
#include "gdiplus.h"


namespace ww{

class FilterEntry : public Entry
{
public:
    FilterEntry(PropertyTree* tree)
    : tree_(tree)
    {
    }
protected:
    bool onKeyPress(const KeyPress& key)
    {
        if (key.key == KEY_UP ||
            key.key == KEY_DOWN ||
            key.key == KEY_ESCAPE ||
            key.key == KEY_RETURN)
        {
            PostMessage(tree_->impl()->get(), WM_KEYDOWN, key.key, 0);
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

YASLI_CLASS(Widget, PropertyTree, "PropertyTree")

TreeConfig TreeConfig::defaultConfig;

TreeConfig::TreeConfig()
: immediateUpdate_(true)
, hideUntranslated_(true)
, valueColumnWidth_(.5f)
, filter_(0)
, compact_(false)
, fullRowMode_(false)
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
, filterMode_(false)
{
	(TreeConfig&)*this = defaultConfig;

	_setMinimalSize(0, 0);

	model_ = new PropertyTreeModel();
	model_->setExpandLevels(expandLevels_);
	model_->setUndoEnabled(undoEnabled_);
	model_->setFullUndo(fullUndo_);

	model_->signalUpdated().connect(this, &PropertyTree::onModelUpdated);

    filterEntry_ = new FilterEntry(this);
    filterEntry_->_setParent(this);
    filterEntry_->signalChanged().connect(this, &PropertyTree::onFilterChanged);

	drawingInit();
}
#pragma warning(pop)

PropertyTree::~PropertyTree()
{
	drawingFinish();
}

void PropertyTree::update()
{
	needUpdate_ = true;
    ::RedrawWindow(impl()->get(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
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
			model()->requestUpdate();
			selectedRow = model()->focusedRow();
		}
		break;
	case KEY_RIGHT:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = ++x);
		if(selectedRow == focusedRow && parentRow->canBeToggled(this) && !parentRow->expanded()){
			expandRow(parentRow, true);
			model()->requestUpdate();
			selectedRow = model()->focusedRow();
		}
		break;
	case KEY_HOME | KEY_MOD_CONTROL:
		selectedRow = model()->root()->rowByVerticalIndex(this, 0);
		if (selectedRow)
			selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		break;
	case KEY_END | KEY_MOD_CONTROL:
		selectedRow = model()->root()->rowByVerticalIndex(this, INT_MAX);
		if (selectedRow)
			selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		break;
	case KEY_HOME:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MIN);
		break;
	case KEY_END:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MAX);
		break;
	case KEY_SPACE:
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
	row = model()->root()->hit(this, point);
	if(row){
		if(!row->isRoot() && row->plusRect().pointInside(point) && impl()->toggleRow(row))
			return true;
		if (row->isSelectable())
			onRowSelected(row, multiSelectable() && Win32::isKeyPressed(KEY_CONTROL), true);	
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
	if(GetCapture() == *_window())
		ReleaseCapture();
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
	typedef std::vector<PropertyRow*> Parents;
	Parents parents;
	PropertyRow* p = row->nonPulledParent()->parent();
	while(p){
		parents.push_back(p);
		p = p->parent();
	}
	Parents::iterator it;
	for(it = parents.begin(); it != parents.end(); ++it)
		expandRow(*it, true);
}

void PropertyTree::expandAll(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		PropertyRow::iterator it;
		FOR_EACH(*root, it){
			PropertyRow* row = *it;
			row->setExpandedRecursive(this, true);
		}
	}
	else
		root->setExpandedRecursive(this, true);
	update();
}

void PropertyTree::collapseAll(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		PropertyRow::iterator it;
		FOR_EACH(*root, it){
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

	update();
}


void PropertyTree::expandRow(PropertyRow* row, bool expanded)
{
	row->_setExpanded(expanded);

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
	needUpdate_ = true;
}

void PropertyTree::interruptDrag()
{
	impl()->drag_.interrupt();
}

void PropertyTree::updateHeights()
{
	needUpdate_ = false;
	model()->root()->calculateMinimalSize(this);
	impl()->size_.y = 0;

    int padding = compact_ ? 0 : 4;

    Rect rect(Vect2(padding, padding), impl()->area_.size() - Vect2(padding, padding) * 2);

	int extraSize = 0;
	model()->root()->adjustRect(this, rect, rect.leftTop(), impl()->size_.y, extraSize);
	impl()->updateScrollBar();
}

Vect2 PropertyTree::treeSize() const
{
	return impl()->size_ + (compact() ? Vect2::ZERO : Vect2(8, 8));
}

void PropertyTree::serialize(Archive& ar)
{
	__super::serialize(ar);

	if(ar.filter(SERIALIZE_DESIGN)){
		//ar(model_, "model", "Model");
		if(ar.isInput())
			update();
	}
	if(ar.filter(SERIALIZE_STATE)){
		bool focused = hasFocus();
		ar(focused, "focused", 0);
		ar(impl()->offset_, "offset", 0);		
		model()->serialize(ar, this);

		if(ar.isInput()){
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
	RedrawWindow(*impl(), 0, 0, RDW_INVALIDATE);
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

void PropertyTree::attach(Serializer serializer)
{
	attached_.clear();
	attached_.push_back(serializer);
	revert();
}

void PropertyTree::attach(Serializers& serializers)
{
	attached_ = serializers;
	revert();
}

void PropertyTree::detach()
{
	if(widget_)
		widget_ = 0;
	attached_.clear();
	model()->root()->clear();
	update();
}

void PropertyTree::revert()
{
	interruptDrag();
	widget_ = 0;

	if(!attached_.empty()){
		PropertyOArchive oa(model_);
		oa.setFilter(filter_);

		Serializers::iterator it = attached_.begin();
		(*it)(oa, "", "");
		while(++it != attached_.end()){
			PropertyTreeModel model2;
			PropertyOArchive oa2(&model2);
			Archive::Context<ww::PropertyTree> treeContext(oa2, this);
			oa2.setFilter(filter_);
			(*it)(oa2, "", "");
			model_->root()->intersect(model2.root());
		}
	}
	else
		model_->clear();

	if (filterMode_)
	{
		model_->root()->updateLabel();		
		onFilterChanged();
	}

	update();
	updateAttachedPropertyTree();
}

void PropertyTree::apply()
{
	if(!attached_.empty()){
		Serializers::iterator it;
		FOR_EACH(attached_, it){
			PropertyIArchive ia(model_);
			Archive::Context<ww::PropertyTree> treeContext(ia, this);
			ia.setFilter(filter_);
			(*it)(ia, "", "");
		}
	}
}

bool PropertyTree::spawnWidget(PropertyRow* row, bool ignoreReadOnly)
{
	if(!widget_ || widget_->row() != row){
		interruptDrag();
		setWidget(0);
		PropertyRowWidget* newWidget = ignoreReadOnly && row->userReadOnlyRecurse() || !row->userReadOnly() ? row->createWidget(this) : 0;
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

bool PropertyTree::onContextMenu(PropertyRow* row, PopupMenuItem& menu)
{
	PropertyRow::iterator it;
	for(it = row->begin(); it != row->end(); ++it){
		PropertyRow* child = *it;
		if(child->isContainer() && child->pulledUp())
			child->onContextMenu(menu, this);
	}
	row->onContextMenu(menu, this);
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
	menu.add(TRANSLATE("Copy"), row)
		.connect(this, &PropertyTree::onRowMenuCopy)
		.setHotkey(KeyPress(Key('C'), KEY_MOD_CONTROL));
    if(!row->userReadOnly()){
	    menu.add(TRANSLATE("Paste"), row)
		    .connect(this, &PropertyTree::onRowMenuPaste)
		    .enable(!row->userReadOnly())
		    .setHotkey(KeyPress(Key('V'), KEY_MOD_CONTROL))
		    .enable(canBePasted(row));
    }
	menu.addSeparator();
	PopupMenuItem& filter = menu.add("Filter by");
	{
		wstring nameFilter = L"\"";
		nameFilter += toWideChar(row->labelUndecorated());
		nameFilter += L"\"";
		filter.add((wstring(L"Name:\t") + nameFilter).c_str(), nameFilter).connect(this, &PropertyTree::startFilter);

		wstring valueFilter = L"=\"";
		valueFilter += row->valueAsWString();
		valueFilter += L"\"";
		filter.add((wstring(L"Value:\t") + valueFilter).c_str(), valueFilter).connect(this, &PropertyTree::startFilter);

		wstring typeFilter = L":\"";
		typeFilter += toWideChar(row->typeNameForFilter());
		typeFilter += L"\"";
		filter.add((wstring(L"Type:\t") + typeFilter).c_str(), typeFilter).connect(this, &PropertyTree::startFilter);
	}
#ifndef NDEBUG
	menu.addSeparator();
	menu.add(TRANSLATE("Decompose"), row).connect(this, &PropertyTree::onRowMenuDecompose);
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

void PropertyTree::onRowMenuCopy(PropertyRow* row)
{
	Clipboard clipboard(this, &constStrings_, model());
    clipboard.copy(row);
}

void PropertyTree::onRowMenuPaste(PropertyRow* row)
{
	if(!canBePasted(row))
		return;
	PropertyRow* parent = row->parent();

    model()->push(row);
	Clipboard clipboard(this, &constStrings_, model());
	if(clipboard.paste(row))
		model()->rowChanged(parent ? parent : model()->root());
	else
		ASSERT(0 && "Unable to paste element!"); // TODO: осмысленное сообщение
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

void PropertyTree::onRowMenuDecompose(PropertyRow* row)
{
	SharedPtr<PropertyRow> clonedRow = row->clone();
	Serializer se(clonedRow);
	edit(se, 0, IMMEDIATE_UPDATE, this);
}

void PropertyTree::onModelUpdated()
{
	if(widget_)
		widget_ = 0;

	if(immediateUpdate_){
		apply();
    	signalChanged_.emit();
        if(autoRevert_)
		    revert();
	}

    setFocus();

	update();

	updateAttachedPropertyTree();
    if(!immediateUpdate_)
	    signalChanged_.emit();
}

void PropertyTree::setWidget(PropertyRowWidget* widget)
{
	if(widget_){
		widget_->commit();
		ASSERT(widget_->actualWidget());
		if(widget_->actualWidget())
			widget_->actualWidget()->hide();
		widget_->acquire();
		widget_ = 0;
	}
	widget_ = widget;
	model()->dismissUpdate();
	if(widget){
		ASSERT(widget_->actualWidget());
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
        RedrawWindow(impl()->get(), 0, 0, RDW_INVALIDATE);
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
			ASSERT(w);
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
				//ASSERT(w);
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
	return focusedWindow == impl()->get() || IsChild(impl()->get(), focusedWindow);
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
    ClientToScreen(*impl(), &pt);
    return Vect2(pt.x, pt.y);
}

void PropertyTree::selectByAddress(void* addr)
{
    if(model()->root()){
        PropertyRow* row = model()->root()->findByAddress(addr);
        TreeSelection sel;
        if(row)
            sel.push_back(model()->pathFromRow(row));
        model()->setSelection(sel);
    }
}

std::wstring generateDigest(Serializer& ser)
{
    PropertyTreeModel model;
    PropertyOArchive oa(&model);
    ser(oa);
    return model.root()->digest();
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

void PropertyTree::updateAttachedPropertyTree()
{
	if(attachedPropertyTree_){
		Serializers serializers;
		TreeSelection::const_iterator i;
		FOR_EACH(model()->selection(), i){
			PropertyRow* row = model()->rowFromPath(*i);
			if (!row)
				continue;

			Serializer ser = row->serializer();

			if (!ser) {
				if(row->pulledUp() || row->pulledBefore()) {
					if (PropertyRow* parent = row->parent())
						ser = parent->serializer();
				}
			}
			
			if (ser)
				serializers.push_back(ser);
		}
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
			markChildrenAsBelonging(row, true);
			row->setBelongsToFilteredRow(false);
		}
		else {
			bool belongs = hasMatchingChildren(row);
			row->setBelongsToFilteredRow(belongs);
			if (belongs) {
				tree->expandRow(row);
				for (int i = 0; i < numChildren; ++i) {
					PropertyRow* child = row->childByIndex(i);
					if (child->pulledUp())
						child->setBelongsToFilteredRow(true);
				}
			}
		}

		row->setMatchFilter(matchFilter);
		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	std::string labelStart_;
};



void PropertyTree::RowFilter::parse(const wchar_t* filter)
{
	for (int i = 0; i < NUM_TYPES; ++i) {
		start[i].clear();
		substrings[i].clear();
		tillEnd[i] = false;
	}

	ESCAPE(filter != 0, return);

	vector<wchar_t> filterBuf(filter, filter + wcslen(filter) + 1);
	CharLowerW(&filterBuf[0]);

	const wchar_t* str = &filterBuf[0];

	Type type = NAME;
	while (true)
	{
		bool fromStart = true;
		while (*str == L' ' || *str == L'*') {
			fromStart = false;
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
	ESCAPE(textOriginal, return false);

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
	needUpdate_ = true;
    ::RedrawWindow(impl()->get(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
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

			Gdiplus::SolidBrush br(Gdiplus::Color(255, 192, 0));
			int left = int(boxFull.X + boxStart.Width) - 1;
			int top = int(boxFull.Y);
			int right = int(boxFull.X + boxEnd.Width);
			int bottom = int(boxFull.Y + boxEnd.Height);
			Gdiplus::Rect highlightRect(left, top, right - left, bottom - top);

			fillRoundRectangle(gr, &br, highlightRect, Gdiplus::Color(255, 255, 128), 1);
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

}
