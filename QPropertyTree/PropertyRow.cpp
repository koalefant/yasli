/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyRowContainer.h"
#include "PropertyDrawContext.h"
#include "yasli/ClassFactory.h"
#include "Unicode.h"
#include "Serialization.h"

#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtCore/QObject>
#include "MathUtils.h"

#if 0
# define DEBUG_TRACE(fmt, ...) printf(fmt "\n", __VA_ARGS__)
# define DEBUG_TRACE_ROW(fmt, ...) for(PropertyRow* zzzz = this; zzzz; zzzz = zzzz->parent()) printf(" "); printf(fmt "\n", __VA_ARGS__)
#else
# define DEBUG_TRACE
# define DEBUG_TRACE_ROW
#endif
	

static QColor interpolate(const QColor& a, const QColor& b, float k)
{
	float mk = 1.0f - k;
	return QColor(a.red() * k  + b.red() * mk,
								a.green() * k + b.red() * mk,
								a.blue() * k + b.blue() * mk,
								a.alpha() * k + b.alpha() * mk);
}

// ---------------------------------------------------------------------------

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

PropertyRow::PropertyRow()
{
	parent_ = 0;

	expanded_ = false;
	selected_ = false;
	visible_ = true;
	labelUndecorated_ = 0;
    belongsToFilteredRow_ = false;
    matchFilter_ = true;
	
	pos_ =  QPoint(0, 0);
	size_ = QPoint(-1, -1);
	plusSize_ = 0;
	textPos_ = 0;
	textSizeInitial_ = 0;
	textHash_ = 0;
	textSize_ = 0;
	widgetPos_ = 0;
    widgetSize_ = 0;
	userWidgetSize_ = -1;
	
	name_ = "";
	typeName_ = "";
	
	pulledUp_ = false;
	pulledBefore_ = false;
	hasPulled_ = false;
	userReadOnly_ = false;
	userReadOnlyRecurse_ = false;
	userFullRow_ = false;
	multiValue_ = false;
	
	label_ = "";
	labelChanged_ = true;
	layoutChanged_ = true;
}

PropertyRow::~PropertyRow()
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
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

const PropertyRow* PropertyRow::childByIndex(int index) const
{
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

void PropertyRow::_setExpanded(bool expanded)
{
    expanded_ = expanded;
	int numChildren = children_.size();

	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* child = children_[i];
		if(child->pulledUp())
			child->_setExpanded(expanded);
	}

	layoutChanged_ = true;
	setLayoutChangedToChildren();
	
}

struct SetExpandedOp {
    bool expanded_;
    SetExpandedOp(bool expanded) : expanded_(expanded) {}
    ScanResult operator()(PropertyRow* row, QPropertyTree* tree, int index)
    {
        if(row->canBeToggled(tree))
            row->_setExpanded(expanded_);
        return SCAN_CHILDREN_SIBLINGS;
    }
};

void PropertyRow::setExpandedRecursive(QPropertyTree* tree, bool expanded)
{
	if(canBeToggled(tree))
		_setExpanded(expanded);
	
    SetExpandedOp op(expanded);
    scanChildren(op, tree);
}

int PropertyRow::childIndex(PropertyRow* row)
{
	YASLI_ASSERT(row);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	YASLI_ESCAPE(it != children_.end(), return -1);
	return std::distance(children_.begin(), it);
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

void PropertyRow::add(PropertyRow* row)
{
	children_.push_back(row);
	row->setParent(this);
}

void PropertyRow::addAfter(PropertyRow* row, PropertyRow* after)
{
	iterator it = std::find(children_.begin(), children_.end(), after);
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
		int numChildren = children_.size();
		for (int i = 0; i < numChildren; ++i) {
            PropertyRow* child = children_[i].get();
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
	size_ = row->size_;
	pos_ = row->pos_;
	plusSize_ = row->plusSize_;
	textPos_ = row->textPos_;
	textSizeInitial_ = row->textSizeInitial_;
	textHash_ = row->textHash_;
	textSize_ = row->textSize_;
	widgetPos_ = row->widgetPos_;
	widgetSize_ = row->widgetSize_;
	userWidgetSize_ = row->userWidgetSize_;

    assignRowState(*row, false);
}

void PropertyRow::replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, bool preserveChildren)
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

void PropertyRow::erase(PropertyRow* row)
{
	row->setParent(0);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	YASLI_ASSERT(it != children_.end());
	if(it != children_.end())
		children_.erase(it);
}

void PropertyRow::swapChildren(PropertyRow* row)
{
	children_.swap(row->children_);
	iterator it;
	for( it = children_.begin(); it != children_.end(); ++it)
		(**it).setParent(this);
	for( it = row->children_.begin(); it != row->children_.end(); ++it)
		(**it).setParent(row);
}

void PropertyRow::addBefore(PropertyRow* row, PropertyRow* before)
{
	if(before == 0)
		children_.push_back(row);
	else{
		iterator it = std::find(children_.begin(), children_.end(), before);
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

PropertyRow* PropertyRow::cloneChildren(PropertyRow* result, const PropertyRow* source) const
{
	PropertyRow::const_iterator it;
	for(it = source->begin(); it != source->end(); ++it){
		const PropertyRow* sourceChild = *it;
		result->add(sourceChild->clone());
	}

	return result;
}

void PropertyRow::drawStaticText(QPainter& p, const QRect& widgetRect)
{
}

void PropertyRow::serialize(Archive& ar)
{
	serializeValue(ar);

	ar(ConstStringWrapper(constStrings_, name_), "name", "name");
	ar(ConstStringWrapper(constStrings_, label_), "label", "label");
	ar(ConstStringWrapper(constStrings_, typeName_), "type", "type");
	ar(reinterpret_cast<std::vector<SharedPtr<PropertyRow> >&>(children_), "children", "!^children");	
	if(ar.isInput()){
		labelChanged_ = true;
		layoutChanged_ = true;
		PropertyRow::iterator it;
		for(it = begin(); it != end(); ){
			PropertyRow* row = *it;
			if(row){
				row->setParent(this);
				++it;
			}
			else{
				YASLI_ASSERT("Missing property row");
				it = erase(it);
			}
		}
	}
}

bool PropertyRow::onActivate(QPropertyTree* tree, bool force)
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
	size_t numChildren = children_.size();
	for (size_t i = 0; i < numChildren; ++i) {
		children_[i]->labelChanged_ = true;
		children_[i]->setLabelChangedToChildren();
	}
}

void PropertyRow::setLayoutChangedToChildren()
{
	size_t numChildren = children_.size();
	for (size_t i = 0; i < numChildren; ++i) {
		children_[i]->layoutChanged_ = true;
		children_[i]->setLayoutChangedToChildren();
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

void PropertyRow::updateLabel(const QPropertyTree* tree, int index)
{
	if (!labelChanged_) {
		if (pulledUp_)
		parent()->hasPulled_ = true;
		return;
	}

	hasPulled_ = false;

	int numChildren = children_.size();
	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		row->updateLabel(tree, i);
	}

	parseControlCodes(label_, true);
	visible_ = *labelUndecorated_ != '\0' || userFullRow_ || pulledUp_ || isRoot();

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
	userFullRow_ = false;
	pulledUp_ = false;
	pulledBefore_ = false;
	userReadOnly_ = false;
	userReadOnlyRecurse_ = false;
	userFixedWidget_ = false;
	userWidgetSize_ = -1;

	while(true){
		if(*ptr == '^'){
			if(parent() && !parent()->isRoot()){
				if(pulledUp_)
					pulledBefore_ = true;
				pulledUp_ = true;
				parent()->hasPulled_ = true;

				if(pulledUp() && isContainer())
					parent()->setPulledContainer(this);
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
		else if(*ptr == '~'){
            ResetSerializerOp op;
            scanChildren(op);
		}
		else if(*ptr == '!'){
			if(userReadOnly_)
				userReadOnlyRecurse_ = true;
			userReadOnly_ = true;
		}
		else if(*ptr == '['){
			++ptr;
			PropertyRow::iterator it;
			for(it = children_.begin(); it != children_.end(); ++it)
				(*it)->parseControlCodes(ptr, false);

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

const char* PropertyRow::typeNameForFilter() const
{
	const char* typeName = this->typeName();
#ifdef _MSC_VER
	if (strncmp(typeName, "struct ", 7) == 0)
		typeName += 7;
	else if(strncmp(typeName, "class ", 6) == 0)
		typeName += 6;
	else if(strncmp(typeName, "enum ", 5) == 0)
		typeName += 5;

#endif
	if(strncmp(typeName, "yasli::", 7) == 0)
		typeName += 7;
	else if(strncmp(typeName, "ww::", 4) == 0)
		typeName += 4;
	else if(strncmp(typeName, "std::", 5) == 0)
		typeName += 5;
	
	return typeName;
}

void PropertyRow::updateTextSizeInitial(const QPropertyTree* tree, int index)
{
	char containerLabel[16] = "";
	const char* text = rowText(containerLabel, tree, index);
	if(text[0] == '\0')
		textSizeInitial_ = 0;
	else{
		unsigned hash = calcHash(text);
		const QFont* font = rowFont(tree);
		hash = calcHash(font, hash);
		if(hash != textHash_){
			QFontMetrics fm(*font);
			textSizeInitial_ = fm.width(text) + 3;
			textHash_ = hash;
		}
	}
}

void PropertyRow::calculateMinimalSize(const QPropertyTree* tree, int posX, bool force, int* _extraSize, int index)
{
	PropertyRow* nonPulled = nonPulledParent();
	if (!layoutChanged_ && !force && !nonPulled->layoutChanged_) {
		DEBUG_TRACE_ROW("... skipping size for %s", label());
		return;
	}

	plusSize_ = 0;
	if(isRoot())
		expanded_ = true;
	else{
		if(nonPulledParent()->isRoot() || tree->compact() && nonPulledParent()->parent()->isRoot())
			_setExpanded(true);
		else if(!pulledUp())
			plusSize_ = tree->tabSize();

		if(parent()->pulledUp())
			pulledBefore_ = false;

		if(!visible(tree) && !(isContainer() && pulledUp())){
			size_ = QPoint(0, 0);
			DEBUG_TRACE_ROW("row '%s' got zero size", label());
			layoutChanged_ = false;
			return;
		}
	}

	widgetSize_ = widgetSizeMin();

	updateTextSizeInitial(tree, index);

	size_ = QPoint(textSizeInitial_ + widgetSizeMin(), isRoot() ? 0 : ROW_DEFAULT_HEIGHT + floorHeight());

	pos_.setX(posX);
	posX += plusSize_;

	int freePulledChildren = 0;
	int extraSizeStorage = 0;
	int& extraSize = !pulledUp() || !_extraSize ? extraSizeStorage : *_extraSize;
	if(!pulledUp()){
		int minTextSize = 0;
		int minimalWidth = 0;
		calcPulledRows(&minTextSize, &freePulledChildren, &minimalWidth, tree, index);
		DEBUG_TRACE_ROW("%s minTextSize: %i, minimalWidth: %i", label(), minTextSize, minimalWidth);
		size_.setX(minimalWidth);
		extraSize = (tree->rightBorder() - tree->leftBorder()) - minimalWidth - posX;
		DEBUG_TRACE_ROW("%s extraSize 0: %i", label(), extraSize);

		float textScale = 1.0f;
		bool hideOwnText = false;
		if(extraSize < 0){
			// hide container item text first
			if (parent() && parent()->isContainer()){
				extraSize += textSizeInitial_;
				minTextSize -= textSizeInitial_;
				hideOwnText = true;
			}

			textScale = clamp(1.0f - float(-extraSize) / minTextSize, 0.0f, 1.0f);
		}
		setTextSize(tree, index, textScale);

		if (hideOwnText) {
			textSize_ = 0;
			DEBUG_TRACE_ROW("%s hideOwnText", label());
		}
	}

	DEBUG_TRACE_ROW("%s extraSize 1: %i", label(), extraSize);

	WidgetPlacement widgetPlace = widgetPlacement();

	if(widgetPlace == WIDGET_ICON){
		widgetPos_ = widgetSize_ ? posX : -1000;
		posX += widgetSize_;
		textPos_ = posX;
		posX += textSize_;
	}

	int numChildren = children_.size();
	if (hasPulled_) {
		for (int i = 0; i < numChildren; ++i) {
			PropertyRow* row = children_[i];
			if(row->visible(tree) && row->pulledBefore()){
				row->calculateMinimalSize(tree, posX, force, &extraSize, i);
				posX += row->size_.x();
			}
		}
	}

	if(widgetPlace != WIDGET_ICON){
		textPos_ = posX;
		posX += textSize_;
	}

	if(widgetPlace == WIDGET_AFTER_NAME){
		widgetPos_ = posX;
		posX += widgetSize_;
	}

	if(widgetPlace == WIDGET_VALUE || freePulledChildren > 0){
		if(!pulledUp() && extraSize > 0){
			// align widget value to value column
			if(!isFullRow(tree))
			{
				int oldX = posX;
				int newX = max(tree->leftBorder() + round((tree->rightBorder() - tree->leftBorder())* (1.f - tree->valueColumnWidth())), posX);
				int xDelta = newX - oldX;
				if (xDelta <= extraSize)
				{
					extraSize -= xDelta;
					posX = newX;
				}
				else
				{
					posX += extraSize;
					extraSize = 0;
				}
			}
		}
	}
	if (freePulledChildren > 0)
		extraSize = extraSize / freePulledChildren;

	if (widgetPlace == WIDGET_VALUE) {
		if(widgetSize_ && !isWidgetFixed() && extraSize > 0) {
			DEBUG_TRACE_ROW("%s widget extraSize: %i", label(), extraSize);
			widgetSize_ += extraSize;
		}

		widgetPos_ = posX;
		DEBUG_TRACE_ROW("textSize: %i widgetPos: %i", int(textSize_), int(widgetPos_));
		posX += widgetSize_;
		int delta = tree->rightBorder() - posX;
		if(delta > 0 && delta < 4)
			widgetSize_ += delta;
	}

	size_.setX(textSize_ + widgetSize_);

	//PropertyRow* nonPulled = nonPulledParent();
	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		if(!row->visible(tree)) {
			DEBUG_TRACE_ROW("skipping invisible child: %s", row->label());
			continue;
		}
		if(row->pulledUp()){
			if(!row->pulledBefore()){
				row->calculateMinimalSize(tree, posX, force, &extraSize, i);
				posX += row->size_.x();
			}
			size_.setX(size_.x() + row->size_.x());
			size_.setY(max(size_.y(), row->size_.y()));
		}
		else if(expanded())
			row->calculateMinimalSize(tree, nonPulled->plusRect().right(), force, &extraSize, i);
	}

	if(!pulledUp())
		size_.setX(tree->rightBorder() - pos_.x());
	DEBUG_TRACE_ROW("calculateMinimalSize: '%s' %i %i (%s)", label(), size_.x(), size_.y(), isRoot() ? "root" : "non-root");
	layoutChanged_ = false;
}

void PropertyRow::adjustVerticalPosition(const QPropertyTree* tree, int& totalHeight)
{
	pos_.setY(totalHeight);
	if(!pulledUp())
		totalHeight += size_.y();
	else{
		pos_.setY(parent()->pos_.y());
		expanded_ = parent()->expanded();
	}
	PropertyRow* nonPulled = nonPulledParent();

	DEBUG_TRACE_ROW("adjustRect: %s %i %i %i %i, totalHeight: %i %s", label(), pos_.x(), pos_.y(), size_.x(), size_.y(), totalHeight, pulledUp() ? "pulled" : "");

	if (expanded_ || hasPulled_) {
		for(PropertyRows::iterator it = children_.begin(); it != children_.end(); ++it){
			PropertyRow* row = *it;
			if(row->visible(tree) && (nonPulled->expanded() || row->pulledUp()))
				row->adjustVerticalPosition(tree, totalHeight);
			else {
				//DEBUG_TRACE_ROW("skip adjust rect for ''", row->label());
			}
		}
	}
}

void PropertyRow::setTextSize(const QPropertyTree* tree, int index, float mult)
{
	updateTextSizeInitial(tree, index);

	textSize_ = round(textSizeInitial_ * mult);

	Rows::iterator i;
	for(i = children_.begin(); i != children_.end(); ++i)
		if((*i)->pulledUp())
			(*i)->setTextSize(tree, 0, mult);
}

void PropertyRow::calcPulledRows(int* minTextSize, int* freePulledChildren, int* minimalWidth, const QPropertyTree *tree, int index) 
{
	updateTextSizeInitial(tree, index);

	*minTextSize += textSizeInitial_;
	if(widgetPlacement() == WIDGET_VALUE && !isWidgetFixed())
		*freePulledChildren += 1;
	*minimalWidth += textSizeInitial_ + widgetSizeMin();

	for(Rows::const_iterator it = children_.begin(); it != children_.end(); ++it)
		if((*it)->pulledUp())
			(*it)->calcPulledRows(minTextSize, freePulledChildren, minimalWidth, tree, index);
}

PropertyRow* PropertyRow::findSelected()
{
    if(selected())
        return this;
    iterator it;
    for(it = children_.begin(); it != children_.end(); ++it){
        PropertyRow* result = (*it)->findSelected();
        if(result)
            return result;
    }
    return 0;
}

PropertyRow* PropertyRow::find(const char* name, const char* nameAlt, const char* typeName)
{
	iterator it;
	for(it = children_.begin(); it != children_.end(); ++it){
		PropertyRow* row = *it;
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
		   ((nameAlt == 0) || (row->label() != 0 && strcmp(row->label(), nameAlt) == 0)) &&
		   ((typeName == 0) || (row->typeName() != 0 && strcmp(row->typeName(), typeName) == 0)))
			return row;
	}
	return 0;
}

PropertyRow* PropertyRow::findFromIndex(int* outIndex, const char* name, const char* typeName, int startIndex) const
{
	int numChildren = children_.size();

	for (int i = startIndex; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		if(((row->name() == name) || strcmp(row->name(), name) == 0) &&
			((row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0))) {
			*outIndex = i;
			return row;
		}
	}

	for (int i = 0; i < startIndex; ++i) {
		PropertyRow* row = children_[i];
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


bool PropertyRow::onKeyDown(QPropertyTree* tree, const QKeyEvent* ev)
{
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parent());
		ContainerMenuHandler menuHandler(tree, container);
		menuHandler.element = this;
		if(ev->key() == Qt::Key_Delete && ev->modifiers() == Qt::NoModifier) {
			menuHandler.onMenuChildRemove();
			return true;
		}
		else if(ev->key() == Qt::Key_Insert && ev->modifiers() == Qt::SHIFT){
			menuHandler.onMenuChildInsertBefore();
			return true;
		}
	}
	return false;
}

bool PropertyRow::onContextMenu(QMenu &menu, QPropertyTree* tree)
{
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parent());
		ContainerMenuHandler* handler = new ContainerMenuHandler(tree, container);
		handler->element = this;
		tree->addMenuHandler(handler);
		if(!container->isFixedSize()){
		    if(!menu.isEmpty())
			    menu.addSeparator();

				QAction* insertBefore = menu.addAction("Insert Before");
				insertBefore->setShortcut(QKeySequence("Shift+Insert"));
				insertBefore->setEnabled(!container->userReadOnly());
				QObject::connect(insertBefore, SIGNAL(triggered()), handler, SLOT(onMenuChildInsertBefore()));
				
				QAction* remove = menu.addAction("Remove");
				remove->setShortcut(QKeySequence("Shift+Delete"));
				remove->setEnabled(!container->userReadOnly());
				QObject::connect(remove, SIGNAL(triggered()), handler, SLOT(onMenuChildRemove()));
		}
	}

	if(hasVisibleChildren(tree)){
		if(!menu.isEmpty())
			menu.addSeparator();

		QObject::connect(menu.addAction("Expand"), SIGNAL(triggered()), tree, SLOT(expandAll()));
		QObject::connect(menu.addAction("Collapse"), SIGNAL(triggered()), tree, SLOT(collapseAll()));
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


const QFont* PropertyRow::rowFont(const QPropertyTree* tree) const
{
	QFont* normalFont = propertyTreeDefaultFont();
	QFont* boldFont = propertyTreeDefaultBoldFont();
	return hasVisibleChildren(tree) || isContainer() ? boldFont : normalFont;
}

void PropertyRow::drawRow(QPainter& painter, const QPropertyTree* tree, int index)
{
	PropertyDrawContext context;
	context.tree = tree;
	context.widgetRect = widgetRect();
	context.lineRect = floorRect();
	context.painter = &painter;
	context.captured = tree->_isCapturedRow(this);

	QColor textColor = tree->palette().buttonText().color();

	QRect rowRect = rect();

	// drawing a horizontal line
	char containerLabel[16] = "";
	yasli::wstring text = toWideChar(rowText(containerLabel, tree, index));

	if(textSize_ && !isStatic() && widgetPlacement() == WIDGET_VALUE &&
		 !pulledUp() && !isFullRow(tree) && !hasPulled())
	{
		QRect rect(textPos_ - 1, rowRect.bottom() - 2, context.lineRect.width() - (textPos_ - 1), 1);

		QLinearGradient gradient(rect.left(), rect.top(), rect.right(), rect.top());
		gradient.setColorAt(0.0f, tree->palette().color(QPalette::Button));
		gradient.setColorAt(0.6f, tree->palette().color(QPalette::Light));
		gradient.setColorAt(0.95f, tree->palette().color(QPalette::Light));
		gradient.setColorAt(1.0f, tree->palette().color(QPalette::Button));
		QBrush brush(gradient);
		painter.fillRect(rect, brush);
	}

	QRect selectionRect = rowRect;
	if(!pulledUp()){
		selectionRect.setLeft(rowRect.left() + plusSize_ - (tree->compact() ? 1 : 2));
		selectionRect.setWidth(rowRect.right() - selectionRect.left());
	}
	else{
		selectionRect.setLeft(rowRect.left() - 1);
		selectionRect.setWidth(rowRect.width() + 1);
		selectionRect.adjust(0, 1, 0, -1);
	}

	if(selected()){
		// drawing a selection rectangle
		QBrush brush(tree->hasFocusOrInplaceHasFocus() ? tree->palette().highlight() : tree->palette().shadow());
		QColor brushColor = brush.color();
		QColor borderColor(brushColor.alpha() / 4, brushColor.red(), brushColor.green(), brushColor.blue());
		fillRoundRectangle(painter, brush, selectionRect, borderColor, 6);
	}
	else 
	{
		bool pulledChildrenSelected = false;

		for (size_t i = 0; i < children_.size(); ++i) {
			PropertyRow* child = children_[i];
			if (!child)
				continue;
			if ((child->pulledBefore() || child->pulledUp()) && child->selected())
				pulledChildrenSelected = true;
		}

		if (pulledChildrenSelected) {
			// draw rectangle around parent of selected pulled row
			QColor color1(tree->palette().button().color());
			QColor color2(tree->hasFocusOrInplaceHasFocus() ? tree->palette().highlight().color() : tree->palette().shadow().color());
			QColor brushColor = interpolate(color1,  color2, 0.22f);
			QBrush brush(brushColor);
			QColor borderColor(brushColor.alpha() / 8, brushColor.red(), brushColor.green(), brushColor.blue());
			fillRoundRectangle(painter, brush, selectionRect, borderColor, 6);
		}
	}

	if(pulledSelected())
		textColor = tree->palette().highlightedText().color();

	if(!tree->compact() || !parent()->isRoot()){
		if(hasVisibleChildren(tree)){
			drawPlus(painter, tree, plusRect(), expanded(), selected(), expanded());
		}
	}

	// custom drawing
	if(!isStatic())
		redraw(context);

	if(textSize_ > 0){
		const QFont* font = rowFont(tree);
		tree->_drawRowLabel(painter, text.c_str(), font, textRect(), textColor);
	}
}




void PropertyRow::drawPlus(QPainter& p, const QPropertyTree* tree, const QRect& rect, bool expanded, bool selected, bool grayed) const
{	
	QPoint size(9, 9);
	QPoint center(rect.center());
	QPoint pt = center - QPoint(4, 4);
	QRect r(pt.x(), pt.y(), size.x(), size.y());

	fillRoundRectangle(p, tree->palette().window(), r, tree->palette().shadow().color(), 3);
	

	QPen pen(tree->palette().shadow().color());
	p.setPen(pen);
	p.drawLine(center.x() - 2, center.y(), center.x() + 2, center.y());
	if (!expanded)
		p.drawLine(center.x(), center.y() - 2, center.x(), center.y() + 2);

}

bool PropertyRow::visible(const QPropertyTree* tree) const
{
	if (tree->_isDragged(this))
		return false;
	return ((visible_ || !tree->hideUntranslated()) && (matchFilter_ || belongsToFilteredRow_));
}

bool PropertyRow::canBeToggled(const QPropertyTree* tree) const
{
	if(!visible(tree))
		return false;
	if((tree->compact() && (parent() && parent()->isRoot())) || isContainer() && pulledUp() || !hasVisibleChildren(tree))
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

bool PropertyRow::canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const QPropertyTree* tree) const
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

void PropertyRow::dropInto(PropertyRow* parentRow, PropertyRow* cursorRow, QPropertyTree* tree, bool before)
{
	SharedPtr<PropertyRow> ref(this);

	PropertyTreeModel* model = tree->model();
	if(parentRow->pulledContainer())
		parentRow = parentRow->pulledContainer();
	if(parentRow->isContainer()){
        tree->model()->push(tree->model()->root()); // FIXME: select optimal row
		setSelected(false);
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parentRow);
		PropertyRow* oldParent = parent();
		TreePath oldParentPath = tree->model()->pathFromRow(oldParent);
		oldParent->erase(this);
		if(before)
			parentRow->addBefore(this, cursorRow);
		else
			parentRow->addAfter(this, cursorRow);
		model->selectRow(this, true);
        TreePath thisPath = tree->model()->pathFromRow(this);
		TreePath parentRowPath = tree->model()->pathFromRow(parentRow);
		if (oldParent = tree->model()->rowFromPath(oldParentPath))
			model->rowChanged(oldParent); // after this call we can get invalid this
		if(PropertyRow* newThis = tree->model()->rowFromPath(thisPath)) {
			TreeSelection selection;
			selection.push_back(thisPath);
			model->setSelection(selection);

			// we use path to obtain new row
            tree->ensureVisible(newThis);
			model->rowChanged(newThis); // after this call row pointers are invalidated
		}
		if (parentRow = tree->model()->rowFromPath(parentRowPath))
			model->rowChanged(parentRow); // after this call row pointers are invalidated
	}
}

void PropertyRow::intersect(const PropertyRow* row)
{
	setMultiValue(multiValue() || row->multiValue() || valueAsString() != row->valueAsString());

	iterator it = begin();
	const_iterator it2 = row->begin();
	for(; it != end();){
		if(it2 == row->end() || strcmp((*it)->typeName(), (*it2)->typeName()) != 0)
			it = children_.erase(it);
		else{
			(*it)->intersect(*it2);
			++it;
			if(it2 != row->end())
				++it2;
		}
	}
}

const char* PropertyRow::rowText(char (&containerLabelBuffer)[16], const QPropertyTree* tree, int index) const
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

bool PropertyRow::hasVisibleChildren(const QPropertyTree* tree, bool internalCall) const
{
	if(empty() || !internalCall && pulledUp())
		return false;

	PropertyRow::const_iterator it;
	for(it = children_.begin(); it != children_.end(); ++it){
		const PropertyRow* child = *it;
		if(child->pulledUp()){
            if(child->hasVisibleChildren(tree, true))
                return true;
        }
        else if(child->visible(tree))
                return true;
	}
	return false;
}

const PropertyRow* PropertyRow::hit(const QPropertyTree* tree, QPoint point) const
{
  return const_cast<PropertyRow*>(this)->hit(tree, point);
}

PropertyRow* PropertyRow::hit(const QPropertyTree* tree, QPoint point)
{
    bool expanded = this->expanded();
    if(isContainer() && pulledUp())
        expanded = parent() ? parent()->expanded() : true;
    bool onlyPulled = !expanded;
    PropertyRow::const_iterator it;
    for(it = children_.begin(); it != children_.end(); ++it){
        PropertyRow* child = *it;
		if (!child->visible(tree))
			continue;
        if(!onlyPulled || child->pulledUp())
            if(PropertyRow* result = child->hit(tree, point))
                return result;
    }
	if (QRect(pos_.x(), pos_.y(), size_.x(), size_.y()).contains(point))
        return this;
    return 0;
}

PropertyRow* PropertyRow::findByAddress(void* addr)
{
    if(serializer_.pointer() == addr)
        return this;
    else{
        Rows::iterator it;
        for(it = children_.begin(); it != children_.end(); ++it){
            PropertyRow* result = it->get()->findByAddress(addr);
            if(result)
                return result;
        }
    }
    return 0;
}

struct GetVerticalIndexOp{
    int index_;
    const PropertyRow* row_;

    GetVerticalIndexOp(const PropertyRow* row) : row_(row), index_(0) {}

    ScanResult operator()(PropertyRow* row, QPropertyTree* tree, int index)
    {
        if(row == row_)
            return SCAN_FINISHED;
        if(row->visible(tree) && row->isSelectable() && !row->pulledUp())
            ++index_;
        return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
    }
};

int PropertyRow::verticalIndex(QPropertyTree* tree, PropertyRow* row)
{
    GetVerticalIndexOp op(row);
	scanChildren(op, tree);
	return op.index_;
}


struct RowByVerticalIndexOp{
    int index_;
    PropertyRow* row_;

    RowByVerticalIndexOp(int index) : row_(0), index_(index) {}

    ScanResult operator()(PropertyRow* row, QPropertyTree* tree, int index)
    {
        if(row->visible(tree) && !row->pulledUp() && row->isSelectable()){
            row_ = row;
            if(index_-- <= 0)
                return SCAN_FINISHED;
        }
        return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
    }
};

PropertyRow* PropertyRow::rowByVerticalIndex(QPropertyTree* tree, int index)
{
    RowByVerticalIndexOp op(index);
	scanChildren(op, tree);
	return op.row_;
}

struct HorizontalIndexOp{
    int index_;
    PropertyRow* row_;
    bool pulledBefore_;

    HorizontalIndexOp(PropertyRow* row) : row_(row), index_(0), pulledBefore_(row->pulledBefore()) {}

    ScanResult operator()(PropertyRow* row, QPropertyTree* tree, int index)
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

int PropertyRow::horizontalIndex(QPropertyTree* tree, PropertyRow* row)
{
	if(row == this)
		return 0;
    HorizontalIndexOp op(row);
	if(row->pulledBefore())
		scanChildrenReverse(op, tree);
	else
		scanChildren(op, tree);
	return op.index_;
}

struct RowByHorizontalIndexOp{
    int index_;
    PropertyRow* row_;
    bool pulledBefore_;

    RowByHorizontalIndexOp(int index) : row_(0), index_(index), pulledBefore_(index < 0) {}

    ScanResult operator()(PropertyRow* row, QPropertyTree* tree, int index)
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

PropertyRow* PropertyRow::rowByHorizontalIndex(QPropertyTree* tree, int index)
{
	if(!index)
		return this;
    RowByHorizontalIndexOp op(index);
	if(index < 0)
		scanChildrenReverse(op, tree);
	else
		scanChildren(op, tree);
	return op.row_ ? op.row_ : this;
}

void PropertyRow::redraw(const PropertyDrawContext& context)
{

}

bool PropertyRow::isFullRow(const QPropertyTree* tree) const
{
    if (tree->fullRowMode())
		return true;
	if (parent() && parent()->isContainer())
		return true;
	return userFullRow();
}


YASLI_CLASS(PropertyRow, PropertyRow, "Structure");

// ---------------------------------------------------------------------------

PropertyRowWidget::PropertyRowWidget(PropertyRow* row, QPropertyTree* tree)
: row_(row)
, model_(tree->model())
, tree_(tree)
{
}

PropertyRowWidget::~PropertyRowWidget()
{
	if(actualWidget())
		actualWidget()->setParent(0);
	tree_->setFocus();
}

FORCE_SEGMENT(PropertyRowNumber)
FORCE_SEGMENT(PropertyRowStringList)
/*
FORCE_SEGMENT(PropertyRowBitVector)
FORCE_SEGMENT(PropertyRowDecorators)
FORCE_SEGMENT(PropertyRowFileSelector)
FORCE_SEGMENT(PropertyRowColor)
FORCE_SEGMENT(PropertyRowHotkey)
FORCE_SEGMENT(PropertyRowSlider)
FORCE_SEGMENT(PropertyRowIcon)
*/

