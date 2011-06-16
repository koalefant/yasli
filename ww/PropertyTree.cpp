#include "StdAfx.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyTreeDrawing.h"
#include "ww/Serialization.h"
#include "ww/PropertyTreeModel.h"
#include "ww/TreeImpl.h"
#include "ww/Window.h"

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
, propertyTree_(0)
, autoRevert_(true)
, needUpdate_(true)
{
	(TreeConfig&)*this = defaultConfig;

	_setMinimalSize(Vect2i(0, 0));

	model_ = new PropertyTreeModel();
	model_->setExpandLevels(expandLevels_);
	model_->setUndoEnabled(undoEnabled_);
	model_->setFullUndo(fullUndo_);

	model_->signalUpdated().connect(this, &PropertyTree::onModelUpdated);

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
    ::RedrawWindow(*impl(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
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
	case VK_SPACE:
		if(activateRow(row))
			return true;
		break;
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
	case VK_F2:
		if(selectedRow())
			selectedRow()->onActivate(this, true);
		break;
	case KEY_APPS: {
		PopupMenu menu(300);
		if(onContextMenu(row, menu.root())){
            Win32::Rect rect(row->rect());
			_window()->clientToScreen(rect);
			Vect2i pt(rect.left + (rect.bottom - rect.top), rect.bottom);
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
		selectedRow = model()->root()->rowByVerticalIndex(this, --y);
		selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		break;
	case KEY_DOWN:
		selectedRow = model()->root()->rowByVerticalIndex(this, ++y);
		selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
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
		selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		break;
	case KEY_END | KEY_MOD_CONTROL:
		selectedRow = model()->root()->rowByVerticalIndex(this, INT_INF);
		selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		break;
	case KEY_HOME:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = -INT_INF);
		break;
	case KEY_END:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_INF);
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

bool PropertyTree::onRowLMBDown(PropertyRow* row, const Rect& rowRect, Vect2i point)
{
	row = impl()->rowByPoint(point);
	if(row){
		if(!row->isRoot() && row->plusRect().pointInside(point) && impl()->toggleRow(row))
			return true;
		onRowSelected(row, multiSelectable() && Win32::isKeyPressed(VK_CONTROL), true);	
	}

	PropertyTreeModel::UpdateLock lock = model()->lockUpdate();
	row = impl()->rowByPoint(point);
	if(row && !row->isRoot()){
		bool changed = false;
		bool capture = row->onMouseDown(this, point, changed);
		if(!changed && !widget_){ // FIXME: осмысленный метод для проверки
			if(capture)
				return true;
			else if(row->widgetRect().pointInside(point)){
				if(!row->hasIcon())
					interruptDrag();
				row->onActivate(this, false);
				return false;
			}
		}
	}
	return false;
}

void PropertyTree::onRowLMBUp(PropertyRow* row, const Rect& rowRect, Vect2i point)
{
	row->onMouseUp(this, point);
	if(GetCapture() == *_window())
		ReleaseCapture();
}

void PropertyTree::onRowRMBDown(PropertyRow* row, const Rect& rowRect, Vect2i point)
{
    SharedPtr<PropertyRow> handle = row;
	onRowSelected(row, false, true);	
	PopupMenu menu(300);
	if(onContextMenu(row, menu.root()))
		menu.spawn(this);
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
	model()->root()->updateSize(this);
	impl()->size_.y = 0;
	int extraSize = 0;
	model()->root()->adjustRect(this, impl()->area_, impl()->area_.leftTop(), impl()->size_.y, extraSize);
	impl()->updateScrollBar();
}

Vect2i PropertyTree::treeSize() const
{
	return impl()->size_ + (compact() ? Vect2i::ZERO : Vect2i(8, 8));
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
		bool focused = _focusedWidget() == this;
		ar(focused, "focused", 0);
		ar(impl()->offset_, "offset", 0);		
		model()->serialize(ar, this);

		if(ar.isInput()){
			ensureVisible(model()->focusedRow());
			updatePropertyTree();
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
    if(_ContainerWithWindow::_focusable())
        _ContainerWithWindow::_setFocus();
    else
        Widget::_setFocus();
}

void PropertyTree::onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos)
{
	if(!row->isRoot())
		model()->selectRow(row, !(addSelection && row->selected() && model()->selection().size() > 1), !addSelection);
	ensureVisible(row);
	if(adjustCursorPos)
		cursorX_ = row->nonPulledParent()->horizontalIndex(this, row);
	updatePropertyTree();
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
			oa2.setFilter(filter_);
			(*it)(oa2, "", "");
			model_->root()->intersect(model2.root());
		}
	}
	else
		model_->clear();
	update();
}

void PropertyTree::apply()
{
	if(!attached_.empty()){
		Serializers::iterator it;
		FOR_EACH(attached_, it){
			PropertyIArchive ia(model_);
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
		PropertyRowWidget* newWidget = ignoreReadOnly && row->readOnlyOver() || !row->readOnly() ? row->createWidget(this) : 0;
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
    if(!row->readOnly()){
	    menu.add(TRANSLATE("Paste"), row)
		    .connect(this, &PropertyTree::onRowMenuPaste)
		    .enable(!row->readOnly())
		    .setHotkey(KeyPress(Key('V'), KEY_MOD_CONTROL))
		    .enable(canBePasted(row));
    }
//#ifndef NDEBUG
//	menu.addSeparator();
//	menu.add(TRANSLATE("Decompose"), row).connect(this, &PropertyTree::onRowMenuDecompose);
//#endif
	return true;
}

void PropertyTree::onRowMouseMove(PropertyRow* row, const Rect& rowRect, Vect2i point)
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
	if(destination->readOnly())
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

    if(Widget::_focusable())
	    setFocus();

	update();

	updatePropertyTree();
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
				rect = Rect(rect.leftTop() - impl()->offset_, 
					rect.rightBottom() - impl()->offset_);
				w->_setPosition(rect);
				if(!w->isVisible()){
					w->show();
					if(w->_focusable())
						w->setFocus();
					else{
						//ASSERT(w->_focusable());
					}
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
}

bool PropertyTree::isFocused() const
{
	return GetFocus() == *impl() || (widget_ && GetFocus() == *widget_->actualWidget()->_window());
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

Vect2i PropertyTree::_toScreen(Vect2i point) const
{
    POINT pt = { point.x - impl()->offset_.x, point.y - impl()->offset_.y };
    ClientToScreen(*impl(), &pt);
    return Vect2i(pt.x, pt.y);
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

bool PropertyTree::_focusable() const
{
    if(_ContainerWithWindow::_focusable())
        return true;
    return Widget::_focusable();
}

void PropertyTree::attachPropertyTree(PropertyTree* propertyTree) 
{ 
	if(propertyTree_)
		propertyTree_->signalChanged().disconnect(this);
	propertyTree_ = propertyTree; 
//	propertyTree_->signalChanged().connect(this, &PropertyTree::revert);
	updatePropertyTree(); 
}

void PropertyTree::updatePropertyTree()
{
	if(propertyTree_){
		Serializers serializers;
		TreeSelection::const_iterator i;
		FOR_EACH(model()->selection(), i){
			PropertyRow* row = model()->rowFromPath(*i);
			if(row && row->serializer())
				serializers.push_back(row->serializer());
			else{
				serializers.clear();
				break;
			}
		}
		propertyTree_->attach(serializers);
	}
}


}
