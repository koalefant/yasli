/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyRowContainer.h"
#include "IDrawContext.h"
#include "yasli/ClassFactory.h"
#include "Unicode.h"
#include "Serialization.h"
#include "IUIFacade.h"

#include "yasli/BinArchive.h"

#include "IMenu.h"
#include "MathUtils.h"

#if 0
# define DEBUG_TRACE(fmt, ...) printf(fmt "\n", __VA_ARGS__)
# define DEBUG_TRACE_ROW(fmt, ...) for(PropertyRow* zzzz = this; zzzz; zzzz = zzzz->parent()) printf(" "); printf(fmt "\n", __VA_ARGS__)
#else
# define DEBUG_TRACE(...)
# define DEBUG_TRACE_ROW(...)
#endif
	

inline unsigned calcHash(const char* str, unsigned hash = 5381)
{
	while(*str)
		hash = hash*33 + (unsigned char)*str++;
	return hash;
}

template<class T>
inline unsigned calcHash(const T& t, unsigned hash = 5381)
{
	for (int i = 0; i < sizeof(T); i++)
		hash = hash * 33 + ((unsigned char*)&t)[i];
	return hash;
}

// ---------------------------------------------------------------------------

ConstStringList* PropertyRow::constStrings_ = 0;

PropertyRow::PropertyRow(bool isStruct)
: parent_(0)
, isStruct_(isStruct)
, expanded_(false)
, selected_(false)
, visible_(true)
, labelUndecorated_(0)
, belongsToFilteredRow_(false)
, matchFilter_(true)
, textSizeInitial_(0)
, textHash_(0)
, userWidgetSize_(-1)
, name_("")
, typeName_("")
, pulledUp_(false)
, pulledBefore_(false)
, hasPulled_(false)
, userReadOnly_(false)
, userReadOnlyRecurse_(false)
, userFullRow_(false)
, multiValue_(false)
, userHideChildren_(false)
, label_("")
, labelChanged_(false)
, layoutChanged_(true)
, layoutElement_(0xffffffff)
{
}

PropertyRow::~PropertyRow()
{
}

PropertyRowStruct::~PropertyRowStruct()
{
	size_t count = children_.size();
	for (size_t i = 0; i < count; ++i)
		if (children_[i]->parent() == this)
			children_[i]->setParent(0);
}

void PropertyRow::setNames(const char* name, const char* label, const char* typeName)
{
	name_ = name;
	label_ = label ? label : "";
	typeName_ = typeName;
}

PropertyRow* PropertyRow::childByIndex(int index)
{
	if (isStruct_)
		return asStruct()->childByIndex(index);
	return 0;
}

PropertyRow* PropertyRowStruct::childByIndex(int index)
{
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

const PropertyRow* PropertyRow::childByIndex(int index) const
{
	if (isStruct_)
		return asStruct()->childByIndex(index);
	return 0;
}

const PropertyRow* PropertyRowStruct::childByIndex(int index) const
{
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

void PropertyRow::_setExpanded(bool expanded)
{
    expanded_ = expanded;
	int numChildren = count();

	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* child = childByIndex(i);
		if(child->pulledUp())
			child->_setExpanded(expanded);
	}

	layoutChanged_ = true;
	setLayoutChangedToChildren();
}

struct SetExpandedOp {
    bool expanded_;
    SetExpandedOp(bool expanded) : expanded_(expanded) {}
    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(row->canBeToggled(tree))
            row->_setExpanded(expanded_);
        return SCAN_CHILDREN_SIBLINGS;
    }
};

void PropertyRow::setExpandedRecursive(PropertyTree* tree, bool expanded)
{
	if(canBeToggled(tree))
		_setExpanded(expanded);

	if (isStruct_) {
		SetExpandedOp op(expanded);
		asStruct()->scanChildren(op, tree);
	}
}

int PropertyRow::childIndex(PropertyRow* row)
{
	if (isStruct_)
		return asStruct()->childIndex(row);
	return -1;
}

int PropertyRowStruct::childIndex(PropertyRow* row)
{
	YASLI_ASSERT(row);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	YASLI_ESCAPE(it != children_.end(), return -1);
	return (int)std::distance(children_.begin(), it);
}

bool PropertyRow::isChildOf(const PropertyRow* row) const
{
	const PropertyRow* p = parent();
	while(p){
		if(p == row)
			return true;
		p = p->parent();
	}
	return false;
}

void PropertyRowStruct::add(PropertyRow* row)
{
	children_.push_back(row);
	row->setParent(this);
}

void PropertyRowStruct::addAfter(PropertyRow* row, PropertyRow* after)
{
	Rows::iterator it = std::find(children_.begin(), children_.end(), after);
	if(it != children_.end()){
		++it;
		children_.insert(it, row);
	}
	else{
		children_.push_back(row);
	}

	row->setParent(this);
}

void PropertyRow::assignRowState(const PropertyRow& row, bool recurse)
{
	expanded_ = row.expanded_;
	selected_ = row.selected_;
    if(recurse){
		int numChildren = count();
		for (int i = 0; i < numChildren; ++i) {
            PropertyRow* child = childByIndex(i);
            YASLI_ESCAPE(child, continue);
			int unusedIndex;
            const PropertyRow* rhsChild = row.findFromIndex(&unusedIndex, child->name(), child->typeName(), i);
            if(rhsChild)
                child->assignRowState(*rhsChild, true);
        }
    }
}

void PropertyRow::assignRowProperties(PropertyRow* row)
{
    YASLI_ESCAPE(row, return);
	parent_ = row->parent_;
	
	userReadOnly_ = row->userReadOnly_;
	userReadOnlyRecurse_ = row->userReadOnlyRecurse_;
	userFixedWidget_ = row->userFixedWidget_;
	pulledUp_ = row->pulledUp_;
	pulledBefore_ = row->pulledBefore_;
	textSizeInitial_ = row->textSizeInitial_;
	textHash_ = row->textHash_;
	userWidgetSize_ = row->userWidgetSize_;

    assignRowState(*row, false);
}

void PropertyRowStruct::replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, bool preserveChildren)
{
	Rows::iterator it = std::find(children_.begin(), children_.end(), oldRow);
	YASLI_ASSERT(it != children_.end());
	if(it != children_.end()){
		newRow->assignRowProperties(*it);
		newRow->labelChanged_ = true;
		if(preserveChildren)
			(*it)->swapChildren(newRow);
		*it = newRow;
	}
}

void PropertyRowStruct::erase(PropertyRow* row)
{
	row->setParent(0);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	YASLI_ASSERT(it != children_.end());
	if(it != children_.end())
		children_.erase(it);
}

void PropertyRowStruct::swapChildren(PropertyRow* _row)
{
	PropertyRowStruct* row = _row->asStruct();
	if (!row)
		return;
	children_.swap(row->children_);
	Rows::iterator it;
	for( it = children_.begin(); it != children_.end(); ++it)
		(**it).setParent(this);
	for( it = row->children_.begin(); it != row->children_.end(); ++it)
		(**it).setParent(row);
}

void PropertyRowStruct::addBefore(PropertyRow* row, PropertyRow* before)
{
	if(before == 0)
		children_.push_back(row);
	else{
		Rows::iterator it = std::find(children_.begin(), children_.end(), before);
		if(it != children_.end())
			children_.insert(it, row);
		else
			children_.push_back(row);
	}
	row->setParent(this);
}

yasli::wstring PropertyRow::valueAsWString() const
{
    return toWideChar(valueAsString().c_str());
}

yasli::string PropertyRow::valueAsString() const
{
	return yasli::string();
}

yasli::Serializer PropertyRow::serializer() const
{
	if (isStruct_)
		return asStruct()->serializer();
	return yasli::Serializer();
}

void PropertyRow::setSerializer(const yasli::Serializer& ser)
{
	if (isStruct_)
		asStruct()->setSerializer(ser);
}

SharedPtr<PropertyRow> PropertyRow::clone(ConstStringList* constStrings) const
{
	PropertyRow::setConstStrings(constStrings);
	yasli::BinOArchive oa;
	SharedPtr<PropertyRow> self(const_cast<PropertyRow*>(this));
	oa(self, "row", "Row");

	yasli::BinIArchive ia;
	ia.open(oa);
	SharedPtr<PropertyRow> clonedRow;
	ia(clonedRow, "row", "Row");
	PropertyRow::setConstStrings(0);
	return clonedRow;
}

void PropertyRow::serialize(Archive& ar)
{
	serializeValue(ar);

	ar(ConstStringWrapper(constStrings_, name_), "name", "name");
	ar(ConstStringWrapper(constStrings_, label_), "label", "label");
	ar(ConstStringWrapper(constStrings_, typeName_), "type", "type");
}

void PropertyRowStruct::serialize(Archive& ar)
{
	PropertyRow::serialize(ar);
	ar(reinterpret_cast<std::vector<SharedPtr<PropertyRow> >&>(children_), "children", "!^children");	
	if(ar.isInput()){
		labelChanged_ = true;
		layoutChanged_ = true;
		Rows::iterator it;
		for(it = children_.begin(); it != children_.end(); ){
			PropertyRow* row = *it;
			if(row){
				row->setParent(this);
				++it;
			}
			else{
				YASLI_ASSERT("Missing property row");
				it = children_.erase(it);
			}
		}
	}
}

bool PropertyRow::onActivate(PropertyTree* tree, bool force)
{
    return tree->spawnWidget(this, force);
}

void PropertyRow::setLabelChanged() 
{ 
	for(PropertyRow* row = this; row != 0; row = row->parent())
		row->labelChanged_ = true;
}

void PropertyRow::setLayoutChanged()
{
	layoutChanged_ = true;
}

void PropertyRow::setLabelChangedToChildren()
{
	size_t numChildren = count();
	for (size_t i = 0; i < numChildren; ++i) {
		childByIndex(i)->labelChanged_ = true;
		childByIndex(i)->setLabelChangedToChildren();
	}
}

void PropertyRow::setLayoutChangedToChildren()
{
	size_t numChildren = count();
	for (size_t i = 0; i < numChildren; ++i) {
		childByIndex(i)->layoutChanged_ = true;
		childByIndex(i)->setLayoutChangedToChildren();
	}
}

void PropertyRow::setLabel(const char* label) 
{
	if (!label)
		label = "";
	if (label_ != label) {
		label_ = label;
		setLabelChanged(); 
	}
}

void PropertyRow::updateLabel(const PropertyTree* tree, int index)
{
	if (!labelChanged_) {
		if (pulledUp_)
		parent()->hasPulled_ = true;
		return;
	}

	hasPulled_ = false;

	int numChildren = count();
	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* row = childByIndex(i);
		row->updateLabel(tree, i);
	}

	parseControlCodes(label_, true);
	visible_ = *labelUndecorated_ != '\0' || userFullRow_ || pulledUp_ || isRoot();
	if (userHideChildren_) {
		for (int i = 0; i < numChildren; ++i) {
			PropertyRow* row = childByIndex(i);
			if (row->pulledUp())
				continue;
			row->visible_ = false;
		}
	}

	if(pulledContainer())
		pulledContainer()->_setExpanded(expanded());

	layoutChanged_ = true;
	labelChanged_ = false;
}

struct ResetSerializerOp{
    ScanResult operator()(PropertyRow* row)
    {
        row->setSerializer(Serializer());
        return SCAN_CHILDREN_SIBLINGS;
    }
};

void PropertyRow::parseControlCodes(const char* ptr, bool changeLabel)
{
	if (changeLabel) {
		userFullRow_ = false;
		pulledUp_ = false;
		pulledBefore_ = false;
		userReadOnly_ = false;
		userReadOnlyRecurse_ = false;
		userFixedWidget_ = false;
		userWidgetSize_ = -1;
		userHideChildren_ = false;
	}

	while(true){
		if(*ptr == '^'){
			if(parent() && !parent()->isRoot()){
				if(pulledUp_)
					pulledBefore_ = true;
				pulledUp_ = true;
				parent()->hasPulled_ = true;

				if(pulledUp() && isContainer())
					parent()->setPulledContainer(asStruct());
			}
		}
		else if(*ptr == '+')
			_setExpanded(true);
		else if(*ptr == '<')
			userFullRow_ = true;
		else if(*ptr == '>'){
			userFixedWidget_ = true;
			const char* p = ++ptr;
			while(*p >= '0' && *p <= '9')
				++p;
			if(*p == '>'){
				userWidgetSize_ = atoi(ptr);
				ptr = ++p;
			}
			continue;
		}
		else if(*ptr == '-')
			userHideChildren_ = true;
		else if(*ptr == '~'){
            ResetSerializerOp op;
			if (isStruct_)
				asStruct()->scanChildren(op);
		}
		else if(*ptr == '!'){
			if(userReadOnly_)
				userReadOnlyRecurse_ = true;
			userReadOnly_ = true;
		}
		else if(*ptr == '['){
			++ptr;
			int num = count();
			for(int i = 0; i < num; ++i)
				childByIndex(i)->parseControlCodes(ptr, false);

			int counter = 1;
			while(*ptr){
				if(*ptr == ']' && !--counter)
					break;
				else if(*ptr == '[')
					++counter;
				++ptr;
			}
		}
		else
			break;
		++ptr;
	}

	if (changeLabel)
		labelUndecorated_ = ptr;

	labelChanged();
}

const char* PropertyRow::typeNameForFilter(PropertyTree* tree) const
{
	return typeName();
}

void PropertyRow::updateTextSizeInitial(const PropertyTree* tree, int index)
{
	char containerLabel[16] = "";
	const char* text = rowText(containerLabel, tree, index);
	if(text[0] == '\0') {
		textSizeInitial_ = 0;
		textHash_ = 0;
	}
	else{
		unsigned hash = calcHash(text);
		hash = calcHash(FONT_NORMAL, hash);
		if(hash != textHash_){
			textSizeInitial_ = tree->ui()->textWidth(text, FONT_NORMAL) + 3;
			textHash_ = hash;
		}
	}
}

void PropertyRow::calculateMinimalSize(const PropertyTree* tree, int posX, bool force, int* _extraSize, int index)
{
	PropertyRow* nonPulled = nonPulledParent();
	if (!layoutChanged_ && !force && !nonPulled->layoutChanged_) {
		DEBUG_TRACE_ROW("... skipping size for %s", label());
		return;
	}

	if(isRoot())
		expanded_ = true;
	else{
		if(nonPulled->isRoot() || (tree->compact() && nonPulled->parent()->isRoot()))
			_setExpanded(true);

		if(parent()->pulledUp())
			pulledBefore_ = false;

		if(!visible(tree) && !(isContainer() && pulledUp())){
			DEBUG_TRACE_ROW("row '%s' got zero size", label());
			layoutChanged_ = false;
			return;
		}
	}

	updateTextSizeInitial(tree, index);

	int freePulledChildren = 0;
	int extraSizeStorage = 0;
	int& extraSize = !pulledUp() || !_extraSize ? extraSizeStorage : *_extraSize;
	if(!pulledUp()){
		int minTextSize = 0;
		int minimalWidth = 0;
		calcPulledRows(&minTextSize, &freePulledChildren, &minimalWidth, tree, index);

		float textScale = 1.0f;
		setTextSize(tree, index, textScale);

	}

	WidgetPlacement widgetPlace = widgetPlacement();

	int numChildren = count();
	if (hasPulled_) {
		for (int i = 0; i < numChildren; ++i) {
			PropertyRow* row = childByIndex(i);
			if(row->visible(tree) && row->pulledBefore()){
				row->calculateMinimalSize(tree, posX, force, &extraSize, i);
			}
		}
	}

	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* row = childByIndex(i);
		if(!row->visible(tree)) {
			DEBUG_TRACE_ROW("skipping invisible child: %s", row->label());
			continue;
		}
		if(row->pulledUp()){
			if(!row->pulledBefore()){
				row->calculateMinimalSize(tree, posX, force, &extraSize, i);
			}
		}
		else if(expanded())
			row->calculateMinimalSize(tree, nonPulled->plusRect(tree).right(), force, &extraSize, i);
	}
	layoutChanged_ = false;
}

void PropertyRow::adjustVerticalPosition(const PropertyTree* tree, int& totalHeight)
{
	if(pulledUp()) {
		expanded_ = parent()->expanded();
	}
	PropertyRow* nonPulled = nonPulledParent();

	if (expanded_ || hasPulled_) {
		int num = count();
		for(int i = 0; i < num; ++i){
			PropertyRow* row = childByIndex(i);
			if(row->visible(tree) && (nonPulled->expanded() || row->pulledUp()))
				row->adjustVerticalPosition(tree, totalHeight);
		}
	}
}

void PropertyRow::setTextSize(const PropertyTree* tree, int index, float mult)
{
	updateTextSizeInitial(tree, index);

	size_t numChildren = count();
	for (size_t i = 0; i < numChildren; ++i) {
		PropertyRow* row = childByIndex(i);
		if(row->pulledUp())
			row->setTextSize(tree, 0, mult);
	}
}

void PropertyRow::calcPulledRows(int* minTextSize, int* freePulledChildren, int* minimalWidth, const PropertyTree *tree, int index) 
{
	updateTextSizeInitial(tree, index);

	size_t numChildren = count();
	for (size_t i = 0; i < numChildren; ++i) {
		PropertyRow* row = childByIndex(i);
		if(row->pulledUp())
			row->calcPulledRows(minTextSize, freePulledChildren, minimalWidth, tree, index);
	}
}

PropertyRow* PropertyRow::findSelected()
{
    if(selected())
        return this;
	int num = count();
    for(int i = 0; i < num; ++i){
        PropertyRow* result = childByIndex(i)->findSelected();
        if(result)
            return result;
    }
    return 0;
}

PropertyRow* PropertyRow::find(const char* name, const char* nameAlt, const char* typeName)
{
	int num = count();
	for(int i = 0; i < num; ++i){
		PropertyRow* row = childByIndex(i);
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
		   ((nameAlt == 0) || (row->label() != 0 && strcmp(row->label(), nameAlt) == 0)) &&
		   ((typeName == 0) || (row->typeName() != 0 && strcmp(row->typeName(), typeName) == 0)))
			return row;
	}
	return 0;
}

PropertyRow* PropertyRow::findFromIndex(int* outIndex, const char* name, const char* typeName, int startIndex) const
{
	int numChildren = count();

	for (int i = startIndex; i < numChildren; ++i) {
		PropertyRow* row = (PropertyRow*)childByIndex(i);
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
			((row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0))) {
			*outIndex = i;
			return row;
		}
	}

	for (int i = 0; i < startIndex; ++i) {
		PropertyRow* row = (PropertyRow*)childByIndex(i);
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
			((row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0))) {
			*outIndex = i;
			return row;
		}
	}

	*outIndex = -1;
	return 0;
}

const PropertyRow* PropertyRow::find(const char* name, const char* nameAlt, const char* typeName) const
{
	return const_cast<PropertyRow* const>(this)->find(name, nameAlt, typeName);
}


bool PropertyRow::onKeyDown(PropertyTree* tree, const KeyEvent* ev)
{
	using namespace property_tree;
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parent());
		ContainerMenuHandler menuHandler(tree, container);
		menuHandler.element = this;
		if(ev->key() == KEY_DELETE && ev->modifiers() == 0) {
			menuHandler.onMenuChildRemove();
			return true;
		}
		else if(ev->key() == KEY_INSERT && ev->modifiers() == MODIFIER_SHIFT){
			menuHandler.onMenuChildInsertBefore();
			return true;
		}
	}
	return false;
}

struct ExpansionMenuHandler : PropertyRowMenuHandler
{
	PropertyTree* tree;
	ExpansionMenuHandler(PropertyTree* tree)
	: tree(tree)
	{
	}

	void onMenuExpand()
	{
		tree->expandAll();
	}
	void onMenuCollapse()
	{
		tree->collapseAll();
	}
};

bool PropertyRow::onContextMenu(property_tree::IMenu &menu, PropertyTree* tree)
{
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parent());
		ContainerMenuHandler* handler = new ContainerMenuHandler(tree, container);
		handler->element = this;
		tree->addMenuHandler(handler);
		if(!container->isFixedSize()){
		    if(!menu.isEmpty())
			    menu.addSeparator();

				menu.addAction("Insert Before", "Shift+Insert",
					container->userReadOnly() ? MENU_DISABLED : 0, handler, &ContainerMenuHandler::onMenuChildInsertBefore);
				
				menu.addAction("Remove", "Shift+Delete",
					container->userReadOnly() ? MENU_DISABLED : 0, handler, &ContainerMenuHandler::onMenuChildRemove);
		}
	}

	if(hasVisibleChildren(tree)){
		if(!menu.isEmpty())
			menu.addSeparator();

		ExpansionMenuHandler* handler = new ExpansionMenuHandler(tree);
		menu.addAction("Expand", 0, handler, &ExpansionMenuHandler::onMenuExpand);
		menu.addAction("Collapse", 0, handler, &ExpansionMenuHandler::onMenuCollapse);
		tree->addMenuHandler(handler);
	}

	return !menu.isEmpty();
}

int PropertyRow::level() const
{
    int result = 0;
    const PropertyRow* row = this;
    while(row){
        row = row->parent();
        ++result;
    }
    return result;
}

PropertyRow* PropertyRow::nonPulledParent()
{
	PropertyRow* row = this;
	while(row->pulledUp())
		row = row->parent();
	return row;
}

PropertyRow* PropertyRow::pulledContainer()
{
	if (isStruct_)
		return asStruct()->pulledContainer();
	return 0;
}

const PropertyRow* PropertyRow::pulledContainer() const
{
	if (isStruct_)
		return asStruct()->pulledContainer();
	return 0;
}

bool PropertyRow::pulledSelected() const
{
	if(selected())
		return true;
	const PropertyRow* row = this;
	while(row->pulledUp()){
		row = row->parent();
		if(row->selected())
			return true;
	}
	return false;
}


Font PropertyRow::rowFont(const PropertyTree* tree) const
{
	return hasVisibleChildren(tree) || isContainer() ? FONT_BOLD : FONT_NORMAL;
}

void PropertyRow::drawRow(IDrawContext& context, const PropertyTree* tree, int index, bool selectionPass)
{
	Rect rowRect = rect(tree);
	Rect selectionRect;
	if(!pulledUp())
		selectionRect = rowRect.adjusted(/*plusSize_ - */(tree->compact() ? 1 : 2), -2, 1, 1);
	else
		selectionRect = rowRect.adjusted(-1, 0, 1, -1);

	if (selectionPass) {
		if (selected()) {
			// drawing a selection rectangle
			context.drawSelection(selectionRect, false);
		}
		else{
			bool pulledChildrenSelected = false;

			int num = count();
			for (int i = 0; i < num; ++i) {
				PropertyRow* child = childByIndex(i);
				if (!child)
					continue;
				if ((child->pulledBefore() || child->pulledUp()) && child->selected())
					pulledChildrenSelected = true;
			}

			if (pulledChildrenSelected) {
				context.drawSelection(selectionRect, true);
				// draw rectangle around parent of selected pulled row
			}
		}
	}
	else{
		context.widgetRect = widgetRect(tree);
		context.captured = tree->_isCapturedRow(this);
		context.pressed = tree->_pressedRow() == this;

		// drawing a horizontal line

		if(textSizeInitial_ && !isStatic() && widgetPlacement() == WIDGET_VALUE &&
		   !pulledUp() && !isFullRow(tree) && !hasPulled() && floorHeight() == 0)
		{
			Rect lineRect = floorRect(tree);
			Rect textRect = this->textRect(tree);
			Rect rect(textRect.left() - 1, rowRect.bottom() - 2, lineRect.width() - (textRect.left() - 1), 1);

			context.drawRowLine(rect);
		}
	}
}

void PropertyRow::drawElement(IDrawContext& context, property_tree::RowPart part, const property_tree::Rect& rect, int partSubindex)
{
	switch (part)
	{
	case PART_ROW_AREA:
		if (selected())
			context.drawSelection(rect, true);
		else
		{
			bool pulledChildrenSelected = false;
			int num = count();
			for (int i = 0; i < num; ++i) {
				PropertyRow* child = childByIndex(i);
				if (!child)
					continue;
				if ((child->pulledBefore() || child->pulledUp()) && child->selected())
					pulledChildrenSelected = true;
			}
			if (pulledChildrenSelected)
				context.drawSelection(rect, true);
		}
		break;
	case PART_LABEL:
		if (textSizeInitial_ > 0){
			char containerLabel[16] = "";
			int index = 0;
			yasli::string text = rowText(containerLabel, context.tree, index);
			context.drawLabel(text.c_str(), FONT_NORMAL, rect, pulledSelected());
		}
		break;
	case PART_PLUS:
		if(!context.tree->compact() || !parent()->isRoot()){
			if(hasVisibleChildren(context.tree)){
				context.drawPlus(rect, expanded(), selected(), expanded());
			}
		}
		break;
	case PART_WIDGET:
	case PART_ARRAY_BUTTON:
		{
			context.widgetRect = rect;
			context.captured = context.tree->_isCapturedRow(this);
			context.pressed = context.tree->_pressedRow() == this;
			redraw(context);
		}
		break;
	default:
		break;
	}
}

bool PropertyRow::visible(const PropertyTree* tree) const
{
	if (tree->_isDragged(this))
		return false;
	return (visible_ && (matchFilter_ || belongsToFilteredRow_));
}

bool PropertyRow::canBeToggled(const PropertyTree* tree) const
{
	if(!visible(tree))
		return false;
	if((tree->compact() && (parent() && parent()->isRoot())) || (isContainer() && pulledUp()) || !hasVisibleChildren(tree))
		return false;
	return !empty();
}

bool PropertyRow::canBeDragged() const
{
	if(parent()){
		if(parent()->isContainer())
			return true;
	}
	return false;
}

bool PropertyRow::canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const PropertyTree* tree) const
{
	YASLI_ASSERT(parentRow);

	if(parentRow->pulledContainer())
		parentRow = parentRow->pulledContainer();

	if(parentRow->isContainer()){
		const PropertyRowContainer* container = static_cast<const PropertyRowContainer*>(parentRow);
				
		if((container->isFixedSize() || container->userReadOnly()) && parent() != parentRow)
			return false;

		if(beforeChild && beforeChild->parent() != parentRow)
			return false;

		const PropertyRow* defaultRow = container->defaultRow(tree->model());
		if(defaultRow && strcmp(defaultRow->typeName(), typeName()) == 0)
			return true;
	}
	return false;	
}

void PropertyRow::dropInto(PropertyRowStruct* parentRow, PropertyRow* cursorRow, PropertyTree* tree, bool before)
{
	SharedPtr<PropertyRow> ref(this);

	PropertyTreeModel* model = tree->model();
	PropertyTreeModel::UpdateLock lock = model->lockUpdate();
	if(parentRow->pulledContainer())
		parentRow = parentRow->pulledContainer();
	if(parentRow->isContainer()){
		tree->model()->rowAboutToBeChanged(tree->model()->root()); // FIXME: select optimal row
		setSelected(false);
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parentRow);
		PropertyRowStruct* oldParent = parent();
		TreePath oldParentPath = tree->model()->pathFromRow(oldParent);
		oldParent->erase(this);
		if(before)
			parentRow->addBefore(this, cursorRow);
		else
			parentRow->addAfter(this, cursorRow);
		model->selectRow(this, true);
		TreePath thisPath = tree->model()->pathFromRow(this);
		TreePath parentRowPath = tree->model()->pathFromRow(parentRow);
		oldParent = (PropertyRowStruct*)tree->model()->rowFromPath(oldParentPath);
		if (oldParent)
			model->rowChanged(oldParent); // after this call we can get invalid this
		if(PropertyRow* newThis = tree->model()->rowFromPath(thisPath)) {
			TreeSelection selection;
			selection.push_back(thisPath);
			model->setSelection(selection);

			// we use path to obtain new row
			tree->ensureVisible(newThis);
			model->rowChanged(newThis); // after this call row pointers are invalidated
		}
		parentRow = (PropertyRowStruct*)tree->model()->rowFromPath(parentRowPath);
		if (parentRow)
			model->rowChanged(parentRow); // after this call row pointers are invalidated
	}
}

void PropertyRow::intersect(const PropertyRow* row)
{
	setMultiValue(multiValue() || row->multiValue() || valueAsString() != row->valueAsString());

	int indexSource = 0;
	int num = count();
	for(int i = 0; i < count(); ++i)
	{
		PropertyRow* testRow = childByIndex(i);
		PropertyRow* matchingRow = row->findFromIndex(&indexSource, testRow->name_, testRow->typeName_, indexSource);
		++indexSource;
		if (matchingRow == 0) {
			asStruct()->erase(testRow);
			--i;
		}	
		else {
			testRow->intersect(matchingRow);
		}
	}
}

const char* PropertyRow::rowText(char (&containerLabelBuffer)[16], const PropertyTree* tree, int index) const
{
	if(parent() && parent()->isContainer()){
		if (tree->showContainerIndices()) {
            sprintf(containerLabelBuffer, " %i.", index);
			return containerLabelBuffer;
		}
		else
			return "";
	}
	else
		return labelUndecorated() ? labelUndecorated() : "";
}

bool PropertyRow::hasVisibleChildren(const PropertyTree* tree, bool internalCall) const
{
	if(empty() || (!internalCall && pulledUp()))
		return false;

	int num = count();
	for(int i = 0; i < num; ++i){
		const PropertyRow* child = childByIndex(i);
		if(child->pulledUp()){
            if(child->hasVisibleChildren(tree, true))
                return true;
        }
        else if(child->visible(tree))
                return true;
	}
	return false;
}

PropertyRow* PropertyRow::findByAddress(const void* addr)
{
	if(serializer().pointer() == addr)
		return this;
	else{
		int num = count();
		for(int i = 0; i < num; ++i){
			PropertyRow* result = childByIndex(i)->findByAddress(addr);
			if(result)
				return result;
		}
	}
	return 0;
}

size_t PropertyRow::count() const
{
	if (isStruct_)
		return asStruct()->count();
	return 0;
}

bool PropertyRow::empty() const{
	if (isStruct_)
		return asStruct()->empty();
	return true;
}

bool PropertyRowStruct::empty() const{
	return children_.empty();
}

struct GetVerticalIndexOp{
	int index_;
	const PropertyRow* row_;

	GetVerticalIndexOp(const PropertyRow* row) : row_(row), index_(0) {}

	ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
	{
		if(row == row_)
			return SCAN_FINISHED;
		if(row->visible(tree) && row->isSelectable() && !row->pulledUp())
			++index_;
		return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
	}
};

int PropertyRow::verticalIndex(PropertyTree* tree, PropertyRow* row)
{
	if (isStruct_) {
		GetVerticalIndexOp op(row);
		asStruct()->scanChildren(op, tree);
		return op.index_;
	}
	else 
		return 0;
}


struct RowByVerticalIndexOp{
    int index_;
    PropertyRow* row_;

    RowByVerticalIndexOp(int index) : row_(0), index_(index) {}

    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(row->visible(tree) && !row->pulledUp() && row->isSelectable()){
            row_ = row;
            if(index_-- <= 0)
                return SCAN_FINISHED;
        }
        return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
    }
};

PropertyRow* PropertyRow::rowByVerticalIndex(PropertyTree* tree, int index)
{
    RowByVerticalIndexOp op(index);
	if (isStruct_) {
		asStruct()->scanChildren(op, tree);
		return op.row_;
	}
	else
		return 0;
}

struct HorizontalIndexOp{
    int index_;
    PropertyRow* row_;
    bool pulledBefore_;

    HorizontalIndexOp(PropertyRow* row) : row_(row), index_(0), pulledBefore_(row->pulledBefore()) {}

    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(!row->pulledUp())
            return SCAN_SIBLINGS;
        if(row->visible(tree) && row->isSelectable() && row->pulledUp() && row->pulledBefore() == pulledBefore_){
            index_ += pulledBefore_ ? -1 : 1;
            if(row == row_)
                return SCAN_FINISHED;
        }
        return SCAN_CHILDREN_SIBLINGS;
    }
};

int PropertyRow::horizontalIndex(PropertyTree* tree, PropertyRow* row)
{
	if(row == this)
		return 0;
    HorizontalIndexOp op(row);
	if (isStruct_) {
		if(row->pulledBefore())
			asStruct()->scanChildrenReverse(op, tree);
		else
			asStruct()->scanChildren(op, tree);
	}
	return op.index_;
}

struct RowByHorizontalIndexOp{
    int index_;
    PropertyRow* row_;
    bool pulledBefore_;

    RowByHorizontalIndexOp(int index) : row_(0), index_(index), pulledBefore_(index < 0) {}

    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(!row->pulledUp())
            return SCAN_SIBLINGS;
        if(row->visible(tree) && row->isSelectable() && row->pulledUp() && row->pulledBefore() == pulledBefore_){
            row_ = row;
            if(pulledBefore_ ? ++index_ >= 0 : --index_ <= 0)
                return SCAN_FINISHED;
        }
        return SCAN_CHILDREN_SIBLINGS;
    }
};

PropertyRow* PropertyRow::rowByHorizontalIndex(PropertyTree* tree, int index)
{
	if(!index)
		return this;
    RowByHorizontalIndexOp op(index);
	if (isStruct()) {
		if(index < 0)
			asStruct()->scanChildrenReverse(op, tree);
		else
			asStruct()->scanChildren(op, tree);
	}
	return op.row_ ? op.row_ : this;
}

void PropertyRow::redraw(IDrawContext& context)
{

}

bool PropertyRow::isFullRow(const PropertyTree* tree) const
{
    if (tree->fullRowMode())
		return true;
	if (parent() && parent()->isContainer())
		return true;
	return userFullRow();
}


Rect PropertyRow::rect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_ROW_AREA, 0);
}

Rect PropertyRow::contentRect(const PropertyTree* tree) const
{
	Rect r = tree->findRowRect(this, PART_CONTENT_AREA, 0);
	if (r.height() == 0) {
		Rect rowRect = rect(tree);
		r = Rect(rowRect.left(), rowRect.top(), rowRect.width(), 0);
	}
	return r;
}

Rect PropertyRow::textRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_LABEL, 0);
}

Rect PropertyRow::widgetRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_WIDGET, 0);
}

Rect PropertyRow::plusRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_PLUS, 0);
}

Rect PropertyRow::floorRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_ROW_AREA, 0);
}

PropertyRowStruct* PropertyRow::asStruct()
{
	if (isStruct_)
		return static_cast<PropertyRowStruct*>(this);
	else
		return 0;
}

const PropertyRowStruct* PropertyRow::asStruct() const
{
	if (isStruct_)
		return static_cast<const PropertyRowStruct*>(this);
	else
		return 0;
}

YASLI_CLASS(PropertyRow, PropertyRow, "Row");
YASLI_CLASS(PropertyRow, PropertyRowStruct, "Struct");

// ---------------------------------------------------------------------------


FORCE_SEGMENT(PropertyRowNumber)
FORCE_SEGMENT(PropertyRowStringList)
FORCE_SEGMENT(PropertyRowButton)
FORCE_SEGMENT(PropertyRowHorizontalLine)
/*
FORCE_SEGMENT(PropertyRowIconXPM)
FORCE_SEGMENT(PropertyRowFileOpen)
FORCE_SEGMENT(PropertyRowFileSave)
FORCE_SEGMENT(PropertyRowColor)
FORCE_SEGMENT(PropertyRowBitVector)
FORCE_SEGMENT(PropertyRowDecorators)
FORCE_SEGMENT(PropertyRowFileSelector)
FORCE_SEGMENT(PropertyRowHotkey)
FORCE_SEGMENT(PropertyRowSlider)
*/
