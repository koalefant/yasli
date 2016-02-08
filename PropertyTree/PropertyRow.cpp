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
#include "PropertyTreeStyle.h"
#include "PropertyRowContainer.h"
#include "ValidatorBlock.h"
#include "IDrawContext.h"
#include "yasli/ClassFactory.h"
#include "yasli/Callback.h"
#include "Unicode.h"
#include "Serialization.h"
#include "IUIFacade.h"

#include "yasli/BinArchive.h"

#include "IMenu.h"
#include "MathUtils.h"

#ifndef _MSC_VER
#include <limits.h>
#endif

#if 0
# define DEBUG_TRACE(fmt, ...) printf(fmt "\n", __VA_ARGS__)
# define DEBUG_TRACE_ROW(fmt, ...) for(PropertyRow* zzzz = this; zzzz; zzzz = zzzz->parent()) printf(" "); printf(fmt "\n", __VA_ARGS__)
#else
# define DEBUG_TRACE(...)
# define DEBUG_TRACE_ROW(...)
#endif
	
enum { TEXT_VALUE_SPACING = 3 };

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

PropertyRow::PropertyRow(bool isStruct, bool isContainer)
: parent_(0)
, isStruct_(isStruct)
, isContainer_(isContainer)
, expanded_(false)
, selected_(false)
, visible_(true)
, belongsToFilteredRow_(false)
, matchFilter_(true)
, textSizeInitial_(0)
, textHash_(0)
, userWidgetSize_(-1)
, name_("")
, typeName_("")
, inlined_(false)
, inlinedBefore_(false)
, hasInlinedChildren_(false)
, userReadOnly_(false)
, userRenamable_(false)
, userFullRow_(false)
, multiValue_(false)
, userHideChildren_(false)
, label_("")
, labelChanged_(false)
, layoutElement_(0xffffffff)
, controlCharacterCount_(0)
, heightIncludingChildren_(0)
, packedAfterPreviousRow_(false)
, callback_(0)
, userPackCheckboxes_(false)
, userWidgetToContent_(false)
, hideChildren_(false)
, validatorHasErrors_(false)
, validatorHasWarnings_(false)
, tooltip_("")
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
	if (callback_)
		callback_->release();
	callback_ = 0;
}

void PropertyRow::setNames(const char* name, const char* label, const char* typeName)
{
	name_ = name;
	YASLI_ASSERT(name);
	label_ = label ? label : "";
	YASLI_ASSERT(strlen(typeName));
	typeName_ = typeName;
}

PropertyRow* PropertyRow::childByIndex(int index)
{
	return static_cast<PropertyRowStruct*>(this)->childByIndex(index);
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
		if(child->inlined())
			child->_setExpanded(expanded);
	}
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

int PropertyRow::childIndex(const PropertyRow* row) const
{
	if (isStruct_)
		return asStruct()->childIndex(row);
	return -1;
}

int PropertyRowStruct::childIndex(const PropertyRow* row) const
{
	YASLI_ASSERT(row);
	Rows::const_iterator it = std::find(children_.begin(), children_.end(), row);
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
	userRenamable_ = row->userRenamable_;
	userFixedWidget_ = row->userFixedWidget_;
	inlined_ = row->inlined_;
	inlinedBefore_ = row->inlinedBefore_;
	textSizeInitial_ = row->textSizeInitial_;
	textHash_ = row->textHash_;
	userWidgetSize_ = row->userWidgetSize_;
	userWidgetToContent_ = row->userWidgetToContent_;
	callback_ = row->callback_;
	row->callback_ = 0;

    assignRowState(*row, false);
}

void PropertyRowStruct::replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, PropertyTreeModel* model, bool preserveChildren)
{
	Rows::iterator it = std::find(children_.begin(), children_.end(), oldRow);
	YASLI_ASSERT(it != children_.end());
	if(it != children_.end()){
		newRow->assignRowProperties(*it);
		newRow->labelChanged_ = true;
		if(preserveChildren)
			(*it)->swapChildren(newRow, model);
		*it = newRow;
		if (model)
			model->callRowCallback(newRow);
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

void PropertyRowStruct::swapChildren(PropertyRow* _row, PropertyTreeModel* model)
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
	if (model){
		for(it = children_.begin(); it != children_.end(); ++it){
			PropertyRow* child = *it;
			if (PropertyRow* srcChild = row->find(child->name(), child->label(), child->typeName())) {
				child->setCallback(srcChild->callback_);
				srcChild->setCallback(0);
				model->callRowCallback(child);
			}
		}
	}
}

void PropertyRowStruct::addBefore(PropertyRow* row, PropertyRow* before)
{
	if(before == 0)
		children_.insert(children_.begin(), row);
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
	if (clonedRow)
		clonedRow->setHideChildren(hideChildren_);
	return clonedRow;
}

void PropertyRow::YASLI_SERIALIZE_METHOD(Archive& ar)
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

bool PropertyRow::onActivate(const PropertyActivationEvent& e)
{
	if (e.reason != e.REASON_RELEASE)
	    return e.tree->spawnWidget(this, e.force);
	else
		return false;
}

void PropertyRow::setLabelChanged() 
{ 
	for(PropertyRow* row = this; row != 0; row = row->parent())
		row->labelChanged_ = true;
}

void PropertyRow::setLabelChangedToChildren()
{
	size_t numChildren = count();
	for (size_t i = 0; i < numChildren; ++i) {
		childByIndex(i)->labelChanged_ = true;
		childByIndex(i)->setLabelChangedToChildren();
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

void PropertyRow::setTooltip(const char* tooltip)
{
	tooltip_ = tooltip;
}

bool PropertyRow::setValidatorEntry(int index, int count)
{
	if (index != validatorIndex_ || count != validatorCount_) {
		validatorIndex_ = min(index, 0xffff);
		validatorCount_ = min(count, 0xff);
		validatorsHeight_ = 0;
		return true;
	}
	return false;
}

void PropertyRow::resetValidatorIcons()
{
	validatorHasWarnings_ = false;
	validatorHasErrors_ = false;
}

void PropertyRow::addValidatorIcons(bool hasWarnings, bool hasErrors)
{
	if (hasWarnings )
		validatorHasWarnings_ = true;
	if (hasErrors)
		validatorHasErrors_ = true;
}

void PropertyRow::updateLabel(const PropertyTree* tree, int index, bool parentHidesNonInlineChildren)
{
	if (!labelChanged_) {
		if (inlined_)
		parent()->hasInlinedChildren_ = true;
		return;
	}

	hasInlinedChildren_ = false;

	int numChildren = count();
	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* row = childByIndex(i);
		row->updateLabel(tree, i, hideChildren_);
	}

	parseControlCodes(tree, label_, true);
	bool hiddenByParentFlag = parentHidesNonInlineChildren && !inlined_;
    visible_ = ((labelUndecorated()[0] != '\0') || userFullRow_ || inlined_ || isRoot()) && !hiddenByParentFlag;

	if (userHideChildren_) {
		for (int i = 0; i < numChildren; ++i) {
			PropertyRow* row = childByIndex(i);
			if (row->inlined())
				continue;
			row->visible_ = false;
		}
	}
	// propagateFlagsTopToBottom();

	if(inlinedContainer())
		inlinedContainer()->_setExpanded(expanded());

	labelChanged_ = false;
}

struct ResetSerializerOp{
    ScanResult operator()(PropertyRow* row)
    {
        row->setSerializer(Serializer());
        return SCAN_CHILDREN_SIBLINGS;
    }
};

void PropertyRow::parseControlCodes(const PropertyTree* tree, const char* ptr, bool changeLabel)
{
	const char* startPtr = ptr;
	if (changeLabel) {
		userFullRow_ = false;
		inlined_ = false;
		inlinedBefore_ = false;
		userReadOnly_ = false;
		userRenamable_ = false;
		userFixedWidget_ = false;
		userPackCheckboxes_ = false;
		userWidgetSize_ = -1;
		userHideChildren_ = false;
		userWidgetToContent_ = false;
	}

	while(true){
		if(*ptr == '^'){
			if(parent() && !parent()->isRoot()){
				if(inlined_)
					inlinedBefore_ = true;
				inlined_ = true;
				parent()->hasInlinedChildren_ = true;

				if(inlined() && isContainer())
					parent()->setInlinedContainer(asStruct());
			}
		}
		else if(*ptr == '='){
			userWidgetToContent_ = true;
		}
		else if(*ptr == '+'){
			bool isFirstUpdate = layoutElement_ == 0xffffffff;
			if (isFirstUpdate)
				_setExpanded(true);
		}
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
			if(userReadOnly_) {
				userRenamable_ = true;
				userReadOnly_ = false;
			}
			else
				userReadOnly_ = true;
		}
		else if(*ptr == '|'){
			userPackCheckboxes_ = true;
		}
		else if(*ptr == '['){
			++ptr;
			int num = count();
			for(int i = 0; i < num; ++i) {
				childByIndex(i)->parseControlCodes(tree, ptr, false);
			}

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

	if (isContainer()) {
		// automatically inline children for short arrays
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(this);
		int numChildren = (int)container->count();
		if (container->isFixedSize() && numChildren > 0 && numChildren <= 4) {
			if (container->childByIndex(0)->inlineInShortArrays()) {
				for(int i = 0; i < numChildren; ++i) {
					PropertyRow* child = container->childByIndex(i);
					child->inlined_ = true;
					if (child->label_) {
						child->controlCharacterCount_ = strlen(child->label_);
					}
				}
				inlined_ = true;
				container->setInlined(true);
			}
		}
	}

	if (changeLabel)
		controlCharacterCount_ = min(255, ptr - startPtr);

	labelChanged();
}

yasli::string PropertyRow::typeNameForFilter(PropertyTree* tree) const
{
	return yasli::makePrettyTypeName(typeName());
}

void PropertyRow::updateTextSizeInitial(const PropertyTree* tree, int index, bool fontChanged)
{
	char containerLabel[16] = "";
	const char* text = rowText(containerLabel, tree, index);
	if(text[0] == '\0' || widgetPlacement() == WIDGET_INSTEAD_OF_TEXT) {
		textSizeInitial_ = 0;
		textHash_ = 0;
	}
	else{
		unsigned hash = calcHash(text);
		property_tree::Font font = rowFont(tree);
		hash = calcHash(font, hash);
		if(hash != textHash_ || fontChanged){
			textSizeInitial_ = tree->ui()->textWidth(text, font) + 3;
			textHash_ = hash;
		}
	}
}

void PropertyRow::calculateMinimalSize(const PropertyTree* tree, int posX, bool force, int* _extraSize, int index)
{
	PropertyRow* nonInlined = findNonInlinedParent();

	if(isRoot())
		expanded_ = true;
	else{
		if(nonInlined->isRoot() || (tree->treeStyle().compact && nonInlined->parent()->isRoot()))
			_setExpanded(true);

		if(parent()->inlined())
			inlinedBefore_ = false;

		if(!visible(tree) && !(isContainer() && inlined())){
			DEBUG_TRACE_ROW("row '%s' got zero size", label());
			return;
		}
	}

	updateTextSizeInitial(tree, index, force);

	int freeInlinedChildren = 0;
	int extraSizeStorage = 0;
	int& extraSize = !inlined() || !_extraSize ? extraSizeStorage : *_extraSize;

	int validatorIconsWidth = 0;
	if (validatorHasErrors_)
		validatorIconsWidth += tree->_defaultRowHeight();
	if (validatorHasWarnings_)
		validatorIconsWidth += tree->_defaultRowHeight();

	if(!inlined()){
		int minTextSize = 0;
		int minimalWidth = 0;
		calcInlinedRows(&minTextSize, &freeInlinedChildren, &minimalWidth, tree, index);
	}

	int numChildren = count();
	if (hasInlinedChildren_) {
		for (int i = 0; i < numChildren; ++i) {
			PropertyRow* row = childByIndex(i);
			if(row->visible(tree) && row->inlinedBefore()){
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
		if(row->inlined() || row->inlinedBefore()){
			if(!row->inlinedBefore()){
				row->calculateMinimalSize(tree, posX, force, &extraSize, i);
			}
		}
		else if(expanded())
			row->calculateMinimalSize(tree, nonInlined->plusRect(tree).right(), force, &extraSize, i);
	}
}

void PropertyRow::setTextSize(const PropertyTree* tree, int index, float mult)
{
	updateTextSizeInitial(tree, index, false);

	size_t numChildren = count();
	for (size_t i = 0; i < numChildren; ++i) {
		PropertyRow* row = childByIndex(i);
		if (row->inlined()) {
			row->setTextSize(tree, 0, mult);
		}
	}
}

void PropertyRow::calcInlinedRows(int* minTextSize, int* freeInlinedChildren, int* minimalWidth, const PropertyTree *tree, int index) 
{
	updateTextSizeInitial(tree, index, false);

	size_t numChildren = count();
	for (size_t i = 0; i < numChildren; ++i) {
		PropertyRow* row = childByIndex(i);
		if(row->inlined())
			row->calcInlinedRows(minTextSize, freeInlinedChildren, minimalWidth, tree, index);
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
	int iterations = 1;

	for (int i = startIndex; i < numChildren; ++i) {
		PropertyRow* row = (PropertyRow*)childByIndex(i);
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
			((row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0))) {
			*outIndex = i;
			if (iterations > 1) 
				printf("searching for \"%s\" with %d iterations!\n", name, iterations);
			return row;
		}
		++iterations;
	}

	for (int i = 0; i < startIndex; ++i) {
		PropertyRow* row = (PropertyRow*)childByIndex(i);
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
			((row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0))) {
			*outIndex = i;
			if (iterations > 1) 
				printf("searching for \"%s\" with %d iterations!\n", name, iterations);
			return row;
		}
		++iterations;
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

PropertyRow* PropertyRow::findNonInlinedParent()
{
	PropertyRow* row = this;
	while(row->inlined())
		row = row->parent();
	return row;
}

PropertyRow* PropertyRow::inlinedContainer()
{
	if (isStruct_)
		return asStruct()->inlinedContainer();
	return 0;
}

const PropertyRow* PropertyRow::inlinedContainer() const
{
	if (isStruct_)
		return asStruct()->inlinedContainer();
	return 0;
}

bool PropertyRow::pulledSelected() const
{
	if(selected())
		return true;
	const PropertyRow* row = this;
	while(row->inlined()){
		row = row->parent();
		if(row->selected())
			return true;
	}
	return false;
}


Font PropertyRow::rowFont(const PropertyTree* tree) const
{
	return (hasVisibleChildren(tree) || (isContainer() && !static_cast<const PropertyRowContainer*>(this)->isInlined())) ? property_tree::FONT_BOLD : property_tree::FONT_NORMAL;
}

void PropertyRow::drawElement(IDrawContext& context, property_tree::RowPart part, const property_tree::Rect& rect, int partSubindex)
{
	switch (part)
	{
	case PART_ROW_AREA:
		if (selected())
			context.drawSelection(rect, false);
		else {
			bool pulledChildrenSelected = false;
			int num = count();
			for (int i = 0; i < num; ++i) {
				PropertyRow* child = childByIndex(i);
				if (!child)
					continue;
				if ((child->inlinedBefore() || child->inlined()) && child->selected())
					pulledChildrenSelected = true;
			}
			if (pulledChildrenSelected)
				context.drawSelection(rect, true);
		}
		break;
	case PART_LABEL:
		if (textSizeInitial_ > 0){
			char containerLabel[16] = "";
			int index = parent() ? parent()->childIndex(this) : 0;
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
	return (visible_ && (matchFilter_ || belongsToFilteredRow_));
}

bool PropertyRow::canBeToggled(const PropertyTree* tree) const
{
	if(!visible(tree))
		return false;
	if((tree->treeStyle().compact && (parent() && parent()->isRoot())) || (isContainer() && inlined()) || !hasVisibleChildren(tree))
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

	if(parentRow->inlinedContainer())
		parentRow = parentRow->inlinedContainer();

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
	if(parentRow->inlinedContainer())
		parentRow = parentRow->inlinedContainer();
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
			tree->ensureVisible(newThis, true, true);
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
	if(parent() && parent()->isContainer() && !inlined()){
		if (tree->showContainerIndices()) {
            sprintf(containerLabelBuffer, "%i.", index);
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
	if(empty() || (!internalCall && inlined()))
		return false;

	int num = count();
	for(int i = 0; i < num; ++i){
		const PropertyRow* child = childByIndex(i);
		if(child->inlined()){
            if(child->hasVisibleChildren(tree, true))
                return true;
        }
        else if(child->visible(tree))
                return true;
	}
	return false;
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

PropertyRow* PropertyRow::findByAddress(const void* addr)
{
    if(searchHandle() == addr)
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


const void* PropertyRow::searchHandle() const
{
	return serializer_.pointer();
}

void PropertyRow::redraw(IDrawContext& context)
{

}

bool PropertyRow::isFullRow(const PropertyTree* tree) const
{
    if (tree->fullRowMode())
		return true;
	if (parent() && parent()->isContainer() && tree->fullRowContainers())
		return true;
	return userFullRow();
}


Rect PropertyRow::rect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_ROW_AREA, 0);
}

Rect PropertyRow::childrenRect(const PropertyTree* tree) const
{
	Rect r = tree->findRowRect(this, PART_CHILDREN_AREA, 0);
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

Rect PropertyRow::validatorRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_VALIDATOR, 0);
}

Rect PropertyRow::validatorErrorIconRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_VALIDATOR_ERROR_ICON, 0);
}

Rect PropertyRow::validatorWarningIconRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_VALIDATOR_WARNING_ICON, 0);
}

Rect PropertyRow::plusRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_PLUS, 0);
}

Rect PropertyRow::floorRect(const PropertyTree* tree) const
{
	return tree->findRowRect(this, PART_ROW_AREA, 0);
}

void PropertyRow::setCallback(yasli::CallbackInterface* callback) {
	callback_ = callback;
}

PropertyRowStruct* PropertyRow::asStruct() {
	if (isStruct_)
		return static_cast<PropertyRowStruct*>(this);
	else
		return 0;
}

const PropertyRowStruct* PropertyRow::asStruct() const {
	return const_cast<PropertyRow*>(this)->asStruct();
}

YASLI_CLASS_NAME(PropertyRow, PropertyRow, "PropertyRow", "Structure");

YASLI_API PropertyRowFactory& globalPropertyRowFactory() {
	return PropertyRowFactory::the();
}

YASLI_API yasli::ClassFactory<PropertyRow>& globalPropertyRowClassFactory()
{
	return yasli::ClassFactory<PropertyRow>::the();
}

YASLI_CLASS(PropertyRow, PropertyRow, "Row");
YASLI_CLASS(PropertyRow, PropertyRowStruct, "Struct");

// ---------------------------------------------------------------------------

int RowWidthCache::getOrUpdate(const PropertyTree* tree, const PropertyRow* rowForValue, int extraSpace)
{
	yasli::string value = rowForValue->valueAsString();
	const Font font = rowForValue->rowFont(tree);
	unsigned int newHash = calculateHash(value.c_str());
	newHash = calculateHash(font, newHash);
	if (newHash != valueHash)
	{
		width = tree->ui()->textWidth(value.c_str(), font) + 6 + extraSpace;
		if (width < 24)
			width = 24;
		valueHash = newHash;
	}
	return width;
}

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
