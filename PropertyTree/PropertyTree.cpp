/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/BinArchive.h"
#include "yasli/Pointers.h"
#include "PropertyTree.h"
#include "IDrawContext.h"
#include "Serialization.h"
#include "PropertyTreeModel.h"
#include "PropertyTreeStyle.h"
#include "ValidatorBlock.h"

#include "yasli/ClassFactory.h"

#include "PropertyOArchive.h"
#include "PropertyIArchive.h"
#include "Unicode.h"
#include "Layout.h"

#include <limits.h>
#include "PropertyTreeMenuHandler.h"
#include "IUIFacade.h"
#include "IMenu.h"

#include "MathUtils.h"

#include "PropertyRowObject.h"

// #define PROFILE
#ifdef PROFILE
#include <QElapsedTimer>
#endif

using yasli::Serializers;
using namespace property_tree;

template<class T>
static bool findInSortedVector(const std::vector<T>& vec, const T& value)
{
	typename std::vector<T>::const_iterator it = std::lower_bound(vec.begin(), vec.end(), value);
	if (it == vec.end())
		return false;
	if (*it != value)
		return false;
	return true;
}

static int findParentLayoutElement(const Layout& layout, int startElement)
{
	int element = -1;
	do 
	{
		size_t num = layout.elements.size();
		for (size_t i = 0; i < num; ++i)
		{
			int childrenListIndex = layout.elements[i].childrenList;
			if (childrenListIndex < 0)
				continue;
			const ChildrenList& l = layout.childrenLists[childrenListIndex];
			size_t numChildren = l.children.size();
			for (size_t j = 0; j < numChildren; ++j) {
				if (l.children[j] == element) {
					element = i;
					break;
				}
			}
		}
	} while (element > 0 && !layout.elements[element].focusable);
	return element;
}

static int findNextChildLayoutElement(const Layout& layout, int startElement)
{
	int element = startElement;
	do {
		if (element > 0 && element < layout.elements.size() && layout.elements[element].childrenList >= 0)
			return layout.childrenLists[layout.elements[element].childrenList].children.front();
		size_t num = layout.elements.size();
		for (size_t i = 0; i < num; ++i)
		{
			int childrenListIndex = layout.elements[i].childrenList;
			if (childrenListIndex < 0)
				continue;
			const ChildrenList& l = layout.childrenLists[childrenListIndex];
			size_t numChildren = l.children.size();
			for (size_t j = 0; j < numChildren; ++j) {
				if (l.children[j] == element)
				{
					if (j + 1 < l.children.size())
						element = l.children[j+1];
					else
						element = -1;
					continue;
				}
			}
		}
	} while (element > 0 && !layout.elements[element].focusable);
	return element;
}

static Point distanceInDirection(const Rect& a, Point aCursor, const Rect& b, Point direction)
{
	int aAlong = min(a.topLeft().dot(direction), a.bottomRight().dot(direction));
	int bAlong = min(b.topLeft().dot(direction), b.bottomRight().dot(direction));
	Point perp(direction.y(), -direction.x());
	int distPerp = abs((aCursor - b.clamp(aCursor)).dot(perp));
	return Point(bAlong - aAlong, distPerp);
}

static bool overlapsOnAxis(const Rect& a, const Rect& b, Point axis)
{
	int aTopLeftProj = a.topLeft().dot(axis); 
	int aBottomRightProj = a.bottomRight().dot(axis);
	int minA = min(aTopLeftProj, aBottomRightProj);
	int maxA = max(aTopLeftProj, aBottomRightProj);
	int bTopLeftProj = b.topLeft().dot(axis); 
	int bBottomRightProj = b.bottomRight().dot(axis);
	int minB = min(bTopLeftProj, bBottomRightProj);
	int maxB = max(bTopLeftProj, bBottomRightProj);
	return !(maxB <= minA || maxA <= minB);
}

// Looks for next adjacent rectangle in the layout.
//
// 'cursorPos' is used to keep column when focus goes through narrow->wide->narrow fields:
//
//  |-------------------------|
//  |        |       |        |
//  |-------------------------|  |
//	|                      +  |  |
//  |-------------------------|  V
//  |        |       |        |
//  |-------------------------|
//
static int findNextLayoutElementInDirection(Point* newCursor, const Layout& layout, int element,
											Point cursorPos, Point direction, bool onlyOverlapping)
{
	const vector<Rect>& rectangles = layout.rectangles;
	const Rect& ref = rectangles[element];
	Point cursor = ref.clamp(cursorPos);

	// select first available element in case we had no selection previously
	if (element == -1) {
		for (size_t i = 0; i < layout.elements.size(); ++i) {
			if (!layout.elements[i].focusable)
				continue;
			return (int)i;
		}
	}

	// iterate over all rectangles and choose closest one in 'direction'
	long long minDistance = LLONG_MAX;

	int bestElement = -1;
	for (size_t i = 0; i < rectangles.size(); ++i) {
		if (i == element)
			continue;
		if (!layout.elements[i].focusable)
			continue;
		const Rect& r = rectangles[i];

		if (onlyOverlapping && !overlapsOnAxis(ref, r, Point(-direction.y(), direction.x())))
			continue;
		Point distance = distanceInDirection(ref, cursor, r, direction);
		if (distance.x() <= 0)
			continue;
		// rectangles are compared by distance along direction (distance.x) and then by distance
		// perpendicular to direction (distance.y)
		long long distanceValue = abs(distance.x()) * 0xffffffffull + abs(distance.y());
		if (distanceValue < minDistance) {
			bestElement = i;
			minDistance = distanceValue;
		}
	}

	// 'newCursor' position is always clipped to a focused rectangle
	if (bestElement > 0) {
		*newCursor = rectangles[bestElement].clamp(cursor);
	}

	return bestElement;
}

// Looks for furthest element in layout in 'direction'. Tries to maintain closest position on
// perpendicular axis.
static int findLastLayoutElementInDirection(Point* newCursor, const Layout& layout, int element, Point cursorPos, Point direction, bool onlyOverlapping) {
	const vector<Rect>& rectangles = layout.rectangles;
	const Rect& ref = rectangles[element];
	Point cursor = ref.clamp(cursorPos);

	// iterate over all rectangles and choose closest one in 'direction'
	int minDistance = INT_MIN;
	int minCrossDistance = INT_MAX;

	int bestElement = -1;
	for (size_t i = 0; i < rectangles.size(); ++i) {
		if (i == element)
			continue;
		if (!layout.elements[i].focusable)
			continue;
		const Rect& r = rectangles[i];
		if (onlyOverlapping && !overlapsOnAxis(ref, r, Point(-direction.y(), direction.x())))
			continue;
		Point distance = distanceInDirection(ref, cursor, r, direction);
		if (distance.x() <= 0)
			continue;
		int distanceValue = abs(distance.x());
		if (distanceValue > minDistance) {
			bestElement = i;
			minDistance = distanceValue;
		} else if ( distanceValue == minDistance ) {
			int crossDistance = abs(distance.y());
			if ( crossDistance < minCrossDistance ) {
				minCrossDistance = crossDistance;
				bestElement = i;
			}
		}
	}

	// 'newCursor' position is always clipped to a focused rectangle
	if (bestElement > 0) {
		*newCursor = rectangles[bestElement].clamp(cursor);
	}

	return bestElement;
}

// ---------------------------------------------------------------------------

TreeConfig::TreeConfig()
: immediateUpdate(true)
, hideUntranslated(true)
, valueColumnWidth(.63f)
, filter(YASLI_DEFAULT_FILTER)
, fullRowContainers(true)
, showContainerIndices(true)
, filterWhenType(true)
, sliderUpdateDelay(25)
, undoEnabled(true)
, multiSelection(false)
{
	defaultRowHeight = 22;
	tabSize = defaultRowHeight;
}

TreeConfig TreeConfig::defaultConfig;

// ---------------------------------------------------------------------------

void PropertyTreeMenuHandler::onMenuFilter()
{
	tree->startFilter("");
}

void PropertyTreeMenuHandler::onMenuFilterByName()
{
	tree->startFilter(filterName.c_str());
}

void PropertyTreeMenuHandler::onMenuFilterByValue()
{
	tree->startFilter(filterValue.c_str());
}

void PropertyTreeMenuHandler::onMenuFilterByType()
{
	tree->startFilter(filterType.c_str());
}

void PropertyTreeMenuHandler::onMenuUndo()
{
	tree->model()->undo();
}

void PropertyTreeMenuHandler::onMenuCopy()
{
	tree->copyRow(row);
}

void PropertyTreeMenuHandler::onMenuPaste()
{
	tree->pasteRow(row);
}
// ---------------------------------------------------------------------------

PropertyTreeStyle PropertyTree::defaultTreeStyle_;

PropertyTree::PropertyTree(IUIFacade* uiFacade)
: attachedPropertyTree_(0)
, ui_(uiFacade)
, autoRevert_(true)
, leftBorder_(0)
, rightBorder_(0)
, cursorX_(0)
, focusedLayoutElement_(-1)
, filterMode_(false)

, applyTime_(0)
, revertTime_(0)
, pressPoint_(-1, -1)
, pressDelta_(0, 0)
, pointerMovedSincePress_(false)
, lastStillPosition_(-1, -1)
, pressedRow_(0)
, capturedRow_(0)
, dragCheckMode_(false)
, dragCheckValue_(false)
, archiveContext_(0)
, outlineMode_(false)
, hideSelection_(false)
, zoomLevel_(10)
, validatorBlock_(new ValidatorBlock)
, style_(new PropertyTreeStyle(defaultTreeStyle_))
, defaultRowHeight_(22)
, layout_(new property_tree::Layout())
, persistentFocusedLayoutElement_(new PersistentLayoutElement())
{
	model_.reset(new PropertyTreeModel(this));
	model_->setExpandLevels(config_.expandLevels);
	model_->setUndoEnabled(config_.undoEnabled);
	model_->setFullUndo(config_.fullUndo);
}

PropertyTree::~PropertyTree()
{
	delete layout_;
	layout_ = 0;
	clearMenuHandlers();
	delete ui_;
}

bool PropertyTree::onRowKeyDown(PropertyRow* row, const KeyEvent* ev)
{
	using namespace property_tree;
	PropertyTreeMenuHandler handler;
	handler.row = row;
	handler.tree = this;
    if (row) {
        if(row->onKeyDown(this, ev))
            return true;
    }

	switch(ev->key()){
	case KEY_C:
	if (ev->modifiers() == MODIFIER_CONTROL)
		handler.onMenuCopy();
	return true;
	case KEY_V:
	if (ev->modifiers() == MODIFIER_CONTROL)
		handler.onMenuPaste();
	return true;
	case KEY_Z:
	if (ev->modifiers() == MODIFIER_CONTROL)
		if(model()->canUndo()){
			model()->undo();
			return true;
		}
	break;
	case KEY_F2:
	if (ev->modifiers() == 0) {
		if(selectedRow()) {
			PropertyActivationEvent ev;
			ev.tree = this;
			ev.reason = ev.REASON_KEYBOARD;
			ev.rename = true;
			selectedRow()->onActivate(ev);
		}
	}
	break;
	case KEY_MENU:
	{
		if (ev->modifiers() == 0) {
			std::auto_ptr<property_tree::IMenu> menu(ui()->createMenu());

			if(onContextMenu(row, *menu)){
				Rect rect(row->widgetRect(this));
				if (rect.height() == 0)
					rect = row->textRect(this);
				menu->exec(Point(rect.left(), rect.bottom()));
			}
			return true;
		}
		break;
	}
	}

	PropertyRow* focusedRow = this->focusedRow();
	PropertyRow* parentRow = focusedRow ? focusedRow->findNonInlinedParent() : 0;
	PropertyRow* selectedRow = 0;
	bool addToSelection = false;
	int previousFocusedElement = focusedLayoutElement_;
	Point focusCursor;

	int key = ev->key();
	int modifiers = ev->modifiers();

	if ( key == KEY_UP ) {
		int element = findNextLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(0,-1), false);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
	}
	if ( key == KEY_DOWN ) {
		int element = findNextLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(0,1), false);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
	}
	if ( key == KEY_LEFT ) {
		int element = findNextLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(-1,0), true);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
		else {
			element = findParentLayoutElement(*layout_, focusedLayoutElement_);
			if (element > 0)
				focusedLayoutElement_ = element;
			else {
				// can't move further to the left, collapse row
				PropertyRow* nonInlinedParent = 0;
				if (focusedLayoutElement_ > 0 && focusedLayoutElement_ < layout_->rows.size()) 
					nonInlinedParent = layout_->rows[focusedLayoutElement_]->findNonInlinedParent();
				if(nonInlinedParent && nonInlinedParent->canBeToggled(this) && nonInlinedParent->expanded())
					expandRow(nonInlinedParent, false);
			}
		}
	}
	if ( key == KEY_RIGHT ) {
		int element = findNextChildLayoutElement(*layout_, focusedLayoutElement_);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
		else {
			element = findNextLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(1,0), true);
			if (element > 0) {
				focusedLayoutElement_ = element;
				focusCursor_ = focusCursor;
			}
			else {
				// can't move further, expand row
				PropertyRow* nonInlinedParent = 0;
				if (focusedLayoutElement_ > 0 && focusedLayoutElement_ < layout_->rows.size()) 
					nonInlinedParent = layout_->rows[focusedLayoutElement_]->findNonInlinedParent();
				if (nonInlinedParent && nonInlinedParent->canBeToggled(this) && !nonInlinedParent->expanded())
					expandRow(nonInlinedParent, true);
			}
		}
	}
	if ( key == KEY_HOME && modifiers == 0) {
		// navigate to the top
		int element = findLastLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(0,-1), false);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
	}
	if ( ( key == KEY_HOME && modifiers == MODIFIER_CONTROL ) ||
		 ( key == KEY_LEFT && modifiers == (MODIFIER_CONTROL|MODIFIER_ALT) ) ) {
		// navigate to the beginning of the row
		int element = findLastLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(-1,0), true);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
	}
	if ( ( key == KEY_END && modifiers == MODIFIER_CONTROL ) ||
		 ( key == KEY_RIGHT && modifiers == (MODIFIER_CONTROL|MODIFIER_ALT) ) ) {
		// navigate to the end of the row
		int element = findLastLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(1,0), true);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
	}
	if ( key == KEY_END && modifiers == 0 ) {
		// navigate to the bottom
		int element = findLastLayoutElementInDirection(&focusCursor, *layout_, focusedLayoutElement_, focusCursor_, Point(0, 1), false);
		if (element > 0) {
			focusedLayoutElement_ = element;
			focusCursor_ = focusCursor;
		}
	}
	if ( key == KEY_SPACE && modifiers == MODIFIER_CONTROL ) {
		if (size_t(focusedLayoutElement_) <  layout_->rows.size()) {
			selectedRow = layout_->rows[focusedLayoutElement_];
			addToSelection = true;
		}
	}
	if ( ( key == KEY_SPACE && modifiers == 0 && !config_.filterWhenType ) || 
		 key == KEY_RETURN ) {
		if(focusedRow->canBeToggled(this))
			expandRow(focusedRow, !focusedRow->expanded());
		else {
			PropertyActivationEvent e;
			e.tree = this;
			e.reason = e.REASON_KEYBOARD;
			e.rename = false;
			focusedRow->onActivate(e);
		}
	}
	if (modifiers != MODIFIER_CONTROL && ev->key() != KEY_SPACE &&
		focusedLayoutElement_ != previousFocusedElement && focusedLayoutElement_ > 0) {
		selectedRow = layout_->rows[focusedLayoutElement_];
	}
	if (selectedRow){
		onRowSelected(vector<PropertyRow*>(1, selectedRow), addToSelection, false);	
		return true;
	}
	return false;
}

struct FirstIssueVisitor
{
	ValidatorEntryType entryType_;
	PropertyRow* startRow_;
	PropertyRow* result;

	FirstIssueVisitor(ValidatorEntryType type, PropertyRow* startRow)
	: entryType_(type)
	, startRow_(startRow)
	, result()
	{
	}

	ScanResult operator()(PropertyRow* row, PropertyTree* tree, int)
	{
		if ((row->inlined() || row->inlinedBefore()) && row->findNonInlinedParent() == startRow_)
			return SCAN_SIBLINGS;
		if (row->validatorCount()) {
			if (const ValidatorEntry* validatorEntries = tree->_validatorBlock()->getEntry(row->validatorIndex(), row->validatorCount())) {
				for (int i = 0; i < row->validatorCount(); ++i) {
					const ValidatorEntry* validatorEntry = validatorEntries + i;
					if (validatorEntry->type == entryType_) {
						result = row;
						return SCAN_FINISHED;
					}
				}
			}
		}
		return SCAN_CHILDREN_SIBLINGS;
	}
};

void PropertyTree::jumpToNextHiddenValidatorIssue(bool isError, PropertyRow* start)
{
	FirstIssueVisitor op(isError ? VALIDATOR_ENTRY_ERROR : VALIDATOR_ENTRY_WARNING, start);
	if (start->isStruct()) {
		start->asStruct()->scanChildren(op, this);
	}

	PropertyRow* row = op.result;

	vector<PropertyRow*> parents;
	while (row && row->parent())  {
		parents.push_back(row);
		row = row->parent();
	}
	for (int i = (int)parents.size()-1; i >= 0; --i) {
		if (!parents[i]->visible(this))
			break;
		row = parents[i];
	}
	if (row)
		setSelectedRow(row);

	updateValidatorIcons();
	updateHeights();
}

static void rowsInBetween(vector<PropertyRow*>* rows, PropertyRow* a, PropertyRow* b)
{
	if (!a)
		return;
	if (!b)
		return;
	vector<PropertyRow*> pathA;
	PropertyRow* rootA = a;
	while (rootA->parent()) {
		pathA.push_back(rootA);
		rootA = rootA->parent();
	}

	vector<PropertyRow*> pathB;
	PropertyRow* rootB = b;
	while (rootB->parent()) {
		pathB.push_back(rootB);
		rootB = rootB->parent();
	}

	if (rootA != rootB)
		return;

	const PropertyRow* commonParent = rootA;
	int maxDepth = min((int)pathA.size(), (int)pathB.size());
	for (int i = 0; i < maxDepth; ++i) {
		PropertyRow* parentA = pathA[(int)pathA.size() - 1 - i];
		PropertyRow* parentB = pathB[(int)pathB.size() - 1 - i];
		if (parentA != parentB) {
			int indexA = commonParent->childIndex(parentA);
			int indexB = commonParent->childIndex(parentB);
			int minIndex = min(indexA, indexB);
			int maxIndex = max(indexA, indexB);
			for (int j = minIndex; j <= maxIndex; ++j)
				rows->push_back((PropertyRow*)commonParent->childByIndex(j));
			return;
		}
		commonParent = parentA;
	}
}

struct LevelHit
{
    int hit;
    int focusable;
    int focusableAlternative;
};

static LevelHit hitLayoutElementRecurse(const Layout& layout, int element, Point point)
{
    LevelHit r = { 0 };
	if (size_t(element) >= layout.elements.size())
        return r;
	const LayoutElement& e = layout.elements[element]; 
	if (element == 0 || layout.rectangles[element].contains(point)) {
        r.hit = element;
        if (e.focusable)
            r.focusable = element;
        if (e.childrenList >= 0) {
			const ChildrenList& children = layout.childrenLists[e.childrenList];
			int firstFocusableChild = 0;
			int firstChildHit = 0;
			int focusableChild = 0;

			for (size_t i = 0; i < children.children.size(); ++i)
			{
				int childIndex = children.children[i];
				const LayoutElement& ce = layout.elements[childIndex];
                LevelHit childHit = hitLayoutElementRecurse(layout, childIndex, point);
                if (childHit.hit > 0)
                    r.hit = childHit.hit;
                if (childHit.focusable > 0)
                    r.focusable = childHit.focusable;
                if (childHit.focusableAlternative > 0)
                    r.focusableAlternative = childHit.focusableAlternative;
                else if (ce.focusable)
                    r.focusableAlternative = childIndex;
			}
            return r;
		}
        return r;
    }
    return r;
}

void PropertyTree::hitTest(property_tree::HitResult* r, const Point& point)
{
	r->point = point;
    r->focusableElementIndex = 0;
    LevelHit hit = hitLayoutElementRecurse(*layout_, 1, point);;
    r->elementIndex = hit.hit;
    if (hit.focusable > 0)
        r->focusableElementIndex = hit.focusable;
    else if (hit.focusableAlternative > 0)
        r->focusableElementIndex = hit.focusableAlternative;
	const LayoutElement& element = layout_->elements[r->elementIndex];
	r->row = layout_->rows[r->elementIndex];
	r->part = (RowPart)element.rowPart;
	r->partIndex = element.rowPartSubindex;
}

bool PropertyTree::onRowLMBDown(const HitResult& hit, bool controlPressed, bool shiftPressed)
{
	PropertyRow* row = hit.row;
	Point point = hit.point;
	pressPoint_ = point;
	pointerMovedSincePress_ = false;
	// row = model()->root()->hit(this, point);
	// if(row){
	// 	if (!row->isRoot()) {
	// 		if(row->plusRect(this).contains(point) && toggleRow(row))
	// 			return true;
	// 		if (row->validatorWarningIconRect(this).contains(point)) {
	// 			jumpToNextHiddenValidatorIssue(false, row);
	// 			return true;
	// 		}
	// 		if (row->validatorErrorIconRect(this).contains(point)) {
	// 			jumpToNextHiddenValidatorIssue(true, row);
	// 			return true;
	// 		}

	bool changed = false;

	PropertyTreeModel::UpdateLock lock = model()->lockUpdate();

	switch (hit.part)
	{
	case PART_PLUS:
	{
		if (toggleRow(row))
			return true;
		break;
	}
	case PART_WIDGET:
	{
		DragCheckBegin dragCheck = row->onMouseDragCheckBegin();
		if (dragCheck != DRAG_CHECK_IGNORE) {
			dragCheckValue_ = dragCheck == DRAG_CHECK_SET;
			dragCheckMode_ = true;
			changed = row->onMouseDragCheck(this, dragCheckValue_);
		}
		break;
	}
	default:
	break;
	}

	focusedLayoutElement_ = hit.focusableElementIndex;

	PropertyRow* rowToSelect = row;
	while (rowToSelect && !rowToSelect->isSelectable())
		rowToSelect = rowToSelect->parent();
	if (rowToSelect)
		onRowSelected(vector<PropertyRow*>(1, rowToSelect), multiSelectable() && controlPressed, true);	

	if (!dragCheckMode_) {
		bool capture = row->onMouseDown(this, point, changed);
		if(!changed && !widget_.get()){
			if(capture)
				return true;
			else if(hit.part == PART_WIDGET){
				if(row->widgetPlacement() != PropertyRow::WIDGET_ICON)
					interruptDrag();
				PropertyActivationEvent ev;
				ev.tree = this;
				ev.reason = ev.REASON_PRESS;
                ev.rename = false;
				row->onActivate(ev);
				return false;
			}
		}
	}
	return false;
}

void PropertyTree::onMouseStill()
{
	if (capturedRow_) {
		PropertyDragEvent e;
		e.tree = this;
		e.pos = ui()->cursorPosition();
		e.start = pressPoint_;

		capturedRow_->onMouseStill(e);
		lastStillPosition_ = e.pos;
	}
}

void PropertyTree::onRowLMBUp(const HitResult& hit)
{
	onMouseStill();
	if (capturedRow_)
		capturedRow_->onMouseUp(this, hit.point);

	if ((!capturedRow_ || capturedRow_ == hit.row) &&
		(pressPoint_ - hit.point).manhattanLength() < 1 &&
		hit.part == PART_WIDGET) {
		PropertyActivationEvent ev;
		ev.tree = this;
		ev.reason = ev.REASON_RELEASE;
		ev.rename = false;
		hit.row->onActivate(ev);
	}
}

void PropertyTree::onRowRMBDown(const HitResult& hit)
{
	SharedPtr<PropertyRow> handle = hit.row;
	PropertyRow* menuRow = 0;
	PropertyRow* row = hit.row;
	
	focusedLayoutElement_ = hit.focusableElementIndex;

	PropertyRow* rowToSelect = row;
	while (rowToSelect && !rowToSelect->isSelectable())
		rowToSelect = rowToSelect->parent();
	if (rowToSelect)
		onRowSelected(vector<PropertyRow*>(1, rowToSelect), multiSelectable(), true);	

	if (row->isSelectable()){
		menuRow = row;
	}
	else{
		if (row->parent() && row->parent()->isSelectable())
			menuRow = row->parent();
	}

	if (menuRow) {
		onRowSelected(std::vector<PropertyRow*>(1, menuRow), false, true);	
		std::auto_ptr<property_tree::IMenu> menu(ui()->createMenu());
		clearMenuHandlers();
		if(onContextMenu(menuRow, *menu))
			menu->exec(_toWidget(hit.point));
	}
}

void PropertyTree::expandParents(PropertyRow* row)
{
	storePersistentFocusElement();
	bool hasChanges = false;
	typedef std::vector<PropertyRow*> Parents;
	Parents parents;
	PropertyRow* p = row->findNonInlinedParent()->parent();
	while(p){
		parents.push_back(p);
		p = p->parent();
	}
	Parents::iterator it;
	for(it = parents.begin(); it != parents.end(); ++it) {
		PropertyRow* row = *it;
		if (!row->expanded() || hasChanges) {
			row->_setExpanded(true);
			hasChanges = true;
		}
	}
	if (hasChanges) {
		updateValidatorIcons();
		updateHeights();
		restorePersistentFocusElement();
	}
}

void PropertyTree::expandAll()
{
	expandChildren(0);
}


void PropertyTree::expandChildren(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		int count = root->count();
		for (int i = 0; i < count; ++i){
			PropertyRow* row = root->childByIndex(i);
			row->setExpandedRecursive(this, true);
		}
	}
	else
		root->setExpandedRecursive(this, true);

	storePersistentFocusElement();
	updateHeights();
	restorePersistentFocusElement();
}

void PropertyTree::collapseAll()
{
	collapseChildren(0);
}

void PropertyTree::collapseChildren(PropertyRow* root)
{
	if(!root){
		root = model()->root();

		int count = root->count();
		for (int i = 0; i < count; ++i){
			PropertyRow* row = root->childByIndex(i);
			row->setExpandedRecursive(this, false);
		}
	}
	else
		root->setExpandedRecursive(this, false);

	storePersistentFocusElement();
	updateHeights();
	restorePersistentFocusElement();
}

void PropertyTree::expandRow(PropertyRow* row, bool expanded, bool updateHeights)
{
	bool hasChanges = false;
	if (row->expanded() != expanded) {
		row->_setExpanded(expanded);
		hasChanges = true;
	}

	if (hasChanges && updateHeights) {
		storePersistentFocusElement();
		this->updateHeights();
		restorePersistentFocusElement();
		updateValidatorIcons();
	}
}

Point PropertyTree::treeSize() const
{
	return size_ + (compact() ? Point(0,0) : Point(8, 8));
}

void PropertyTree::YASLI_SERIALIZE_METHOD(Archive& ar)
{
	model()->YASLI_SERIALIZE_METHOD(ar, this);

	if(ar.isInput()){
		updateAttachedPropertyTree(false);
		updateHeights();
		onSelected();
	}
}

static void populateRowArea(bool* hasNonPulledChildren, Layout* l, int rowArea, PropertyRow* row, PropertyTree* tree, int indexForContainerElement, bool isInlined)
{
	PropertyRow::WidgetPlacement placement = row->widgetPlacement();
	int widgetSizeMin = row->widgetSizeMin(tree);
	int labelMin = row->textSizeInitial();
	char labelBuffer[16] = ""; 
	const char* label = row->rowText(labelBuffer, tree, indexForContainerElement);
	ElementType labelElementType = (row->isFullRow(tree) || row->inlined()) ? FIXED_SIZE : EXPANDING_MAGNET;
	int labelPriority = 1;
	int labelElement = -1;
	int widgetElement = -1;

	bool widgetFocusable = row->isSelectable();
	bool labelBeforeInlined = false;
	if (row->parent() && row->parent()->isContainer() && 
		(placement == PropertyRow::WIDGET_VALUE || placement == PropertyRow::WIDGET_NONE)) {
		labelBeforeInlined = true;
		if (label[0])
			labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, false);
	}

	int count = (int)row->count();
	for (size_t j = 0; j < count; ++j) {
		PropertyRow* child = row->childByIndex(j);
		if (!child->visible(tree))
			continue;
		if (child->inlinedBefore()) {
			populateRowArea(hasNonPulledChildren, l, rowArea, child, tree, 0, true);
		}
	}

	switch (placement) {
	case PropertyRow::WIDGET_ICON:
	widgetElement = l->addElement(rowArea, FIXED_SIZE, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusable);
	if (label[0])
		l->addElement(rowArea, isInlined ? FIXED_SIZE : EXPANDING, row, PART_LABEL, labelMin, 0, labelPriority, false);
	break;
	case PropertyRow::WIDGET_VALUE: {
		if (!labelBeforeInlined && label[0])
			labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, false);
		ElementType widgetElementType = row->userFullRow() ? EXPANDING :
										row->userFixedWidget() ? FIXED_SIZE : EXPANDING;
		widgetElement = l->addElement(rowArea, widgetElementType, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusable);
	}
	break;
	case PropertyRow::WIDGET_NONE:
	if (!labelBeforeInlined && label[0])
		labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, false);
	break;
	case PropertyRow::WIDGET_AFTER_NAME:
	if (label[0])
		labelElement = l->addElement(rowArea, FIXED_SIZE, row, PART_LABEL, labelMin, 0, labelPriority, false);
	widgetElement = l->addElement(rowArea, FIXED_SIZE, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusable);
	break;
	case PropertyRow::WIDGET_AFTER_INLINED: {
		if (label[0])
			labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, false);
		// add value later
		break;
	}
	case PropertyRow::WIDGET_INSTEAD_OF_TEXT: {
		
		break;
	}
	}

	for (size_t j = 0; j < count; ++j) {
		PropertyRow* child = row->childByIndex(j);
		if (!child->visible(tree))
			continue;
		if (child->inlined() && !child->inlinedBefore()) {
			populateRowArea(hasNonPulledChildren, l, rowArea, child, tree, 0, true);
		}
		else if (!child->inlinedBefore())
			*hasNonPulledChildren = true;
	}

	if (placement == PropertyRow::WIDGET_AFTER_INLINED) {
		widgetElement = l->addElement(rowArea, FIXED_SIZE, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusable);
	}
		row->setLayoutElement(rowArea);
	if (labelElement > 0 && (*hasNonPulledChildren || (row->parent() && row->parent()->isContainer())))
		l->elements[labelElement].focusable = true;
}


static void populateChildrenArea(Layout* l, int parentElement, PropertyRow* parentRow, PropertyTree* tree)
{
	int rowHeight = tree->_defaultRowHeight();
	// assuming that parentRow is expanded
	int count = (int)parentRow->count();
	for (int i = 0 ; i < count; ++i) {
		PropertyRow* child = parentRow->childByIndex(i);
		if (!child->visible(tree))
			continue;

		// children of inlined elements here
		if (child->inlined() || child->inlinedBefore()) {
			populateChildrenArea(l, parentElement, child, tree);
			continue;
		}

		int rowArea = l->addElement(parentElement, HORIZONTAL, child, PART_ROW_AREA, 0, rowHeight, 0, false);
		bool showPlus = !(tree->compact() && parentRow->isRoot());
		if (showPlus)
			l->addElement(rowArea, FIXED_SIZE, child, PART_PLUS, rowHeight, 0, 0, false);

		bool hasNonPulledChildren = false;
		populateRowArea(&hasNonPulledChildren, l, rowArea, child, tree, i, false);

		if (child->validatorCount()) {
			if (const ValidatorEntry* validatorEntries = tree->_validatorBlock()->getEntry(child->validatorIndex(), child->validatorCount())) {
				for (int i = 0; i < child->validatorCount(); ++i) {
					const ValidatorEntry* validatorEntry = validatorEntries + i;
					l->addElement(parentElement, FIXED_SIZE, child, PART_VALIDATOR, 40, 40, 0, false);
				}
			}
		}

		if (child->expanded()) {
			int indentationAndContent = l->addElement(parentElement, HORIZONTAL, child, PART_INDENTATION_AND_CONTENT_AREA, 0, 0, 0, false);
			if (showPlus)
				l->addElement(indentationAndContent, FIXED_SIZE, child, PART_INDENTATION, 20, 0, 0, false);
			int contentArea = l->addElement(indentationAndContent, VERTICAL, child, PART_CHILDREN_AREA, 0, 0, 0, false);
			populateChildrenArea(l, contentArea, child, tree);
		}
	}
}

void calculateMinimalSizes(int* minWidth, int* minHeight, Layout* l, int element)
{
	LayoutElement& e = l->elements[element];
	if (e.type == FIXED_SIZE || e.type == EXPANDING || e.type == EXPANDING_MAGNET) {
		*minWidth = e.minWidth;
		*minHeight = e.minHeight;
	}
	else {
		int w = 0, h = 0;
		if (e.childrenList != -1) {
			const vector<int>& children = l->childrenLists[e.childrenList].children;
			int count = (int)children.size();
			for (int i = 0; i < count; ++i) {
				int childrenWidth = 0;
				int childrenHeight = 0;
				LayoutElement& child = l->elements[children[i]];
				calculateMinimalSizes(&childrenWidth, &childrenHeight, l, children[i]);
				if (e.type == HORIZONTAL) {
					if (child.priority == 0)
						w += childrenWidth;
					h = max(h, childrenHeight);
				}
				else if (e.type == VERTICAL) {
					// Exception for vertical layouts, so only individual rows
					// are getting outside of window, when it is too narrow.
					//
					// May cause problems with multiple columns.
					//
					// w = max(w, childrenWidth);
					if (child.priority == 0)
						h += childrenHeight;
				}
			}
		}
		*minWidth = max(w, e.minWidth);
		*minHeight = max(h, e.minHeight);
	}
	l->minimalWidths[element] = *minWidth;
	l->minimalHeights[element] = *minHeight;
}

static void calculateRectangles(Layout* l, int element, int width, int height, int left, int top)
{
	LayoutElement& e = l->elements[element];
	l->rectangles[element] = Rect(left, top, width, height);
	if (e.type == HORIZONTAL || e.type == VERTICAL) {
		int availableLength = e.type == HORIZONTAL ? width : height;
		if (e.childrenList != -1) {

			const vector<int>& children = l->childrenLists[e.childrenList].children;
			int count = (int)children.size();
			int expandingCount = 0;
			int countByPriority[MAX_PRIORITY] = { 0 };
			int lengthByPriority[MAX_PRIORITY+1] = { 0 };
			int totalFixedLength = 0;;
			for (int i = 0; i < count; ++i) {
				LayoutElement& child = l->elements[children[i]];
				if (child.type != FIXED_SIZE)
					++expandingCount;
				countByPriority[child.priority] += 1;
				int childFixedLength = e.type == HORIZONTAL ? l->minimalWidths[children[i]] : l->minimalHeights[children[i]];
				lengthByPriority[child.priority] += childFixedLength;
				totalFixedLength += childFixedLength;
			}


			int discardedPriority = MAX_PRIORITY;
			int itemsToDiscardPartially = 0;
			int fixedLength = 0;;//e.type == HORIZONTAL ? l->minimalWidths[element] : l->minimalHeights[element];
			for (int i = 0; i < MAX_PRIORITY; ++i) {
				if (i && fixedLength + lengthByPriority[i] > availableLength) {
					itemsToDiscardPartially = countByPriority[i];
					discardedPriority = i;
					break;
				}
				fixedLength += lengthByPriority[i];
			}
			int freeSpaceLeft = max(0, availableLength - fixedLength);

			int expandingLeft = expandingCount;
			int position = 0;
			for (int i = 0; i < count; ++i) {
				LayoutElement& child = l->elements[children[i]];
				int childFixedLength = e.type == HORIZONTAL ? l->minimalWidths[children[i]] : l->minimalHeights[children[i]];
				int childLength = childFixedLength;
				if (child.priority > discardedPriority) {
					childLength = 0;
				}
				else if (child.priority == discardedPriority) {
					childLength = min(childLength, freeSpaceLeft / itemsToDiscardPartially);
					freeSpaceLeft -= childLength;
					--itemsToDiscardPartially;
				}
				else if(discardedPriority == MAX_PRIORITY) { 
					if (child.type == EXPANDING_MAGNET) {
						int magnetLeft = max(0, l->magnetPoint - (left + position + childLength));
						int freeDelta = min(magnetLeft, freeSpaceLeft);
						childLength += freeDelta;
						freeSpaceLeft -= freeDelta;
					}
					else if (child.type != FIXED_SIZE) {
						int freeDelta = expandingLeft ? freeSpaceLeft / expandingLeft : 0;
						childLength += freeDelta;
						freeSpaceLeft -= freeDelta;
					}
				}
				if (child.type != FIXED_SIZE)
					--expandingLeft;
				Rect r = e.type == HORIZONTAL ? Rect(left + position, top, childLength, height)
							                  : Rect(left, top + position, width, childLength);
				calculateRectangles(l, children[i], r.width(), r.height(), r.left(), r.top());
				position += childLength;
			}

		}
	}
}

static int findRowElement(const Layout& l, const PropertyRow* row, int part, int subindex)
{
	int index = row->layoutElement();
	if (size_t(index) >= l.elements.size())
		return 0;
	for (; index < l.elements.size(); ++index) {
		const LayoutElement& element = l.elements[index];
		if (element.rowPart == part && element.rowPartSubindex == subindex && l.rows[index] == row)
			return index;
	}
	return 0;
}

static int findFocusableRowElement(const Layout& l, const PropertyRow* row)
{
	int index = row->layoutElement();
	if (size_t(index) >= l.elements.size())
		return 0;
	for (; index < l.elements.size(); ++index) {
		const LayoutElement& element = l.elements[index];
		if (l.rows[index] == row && element.focusable)
			return index;
	}
	return 0;
}

Rect PropertyTree::findRowRect(const PropertyRow* row, int part, int subindex) const
{
	int element = findRowElement(*layout_, row, part, subindex);
	if (element > 0)
		return layout_->rectangles[element];
	return Rect();
}

void PropertyTree::updateLayout()
{
	Layout& l = *layout_;
	l.clear();

	int width = area_.width();
	l.magnetPoint = int(width * (1.0f - valueColumnWidth()));

	l.elements.push_back(LayoutElement());
	PropertyRow* root = model_->root();
	l.rows.push_back(root);

	int lroot = l.addElement(-1, VERTICAL, root, PART_CHILDREN_AREA, 0, 0, 0, false);
	root->setLayoutElement(lroot);
	populateChildrenArea(&l, lroot, root, this);

	l.minimalWidths.resize(l.elements.size());
	l.minimalHeights.resize(l.elements.size());

	int minWidth = 0;
	int minHeight = 0;
	calculateMinimalSizes(&minWidth, &minHeight, &l, lroot);

	l.rectangles.resize(l.elements.size());
	calculateRectangles(&l, lroot, width, minHeight, 0, filterAreaHeight());
	printf("Minimal size: %d %d\n", minWidth, minHeight);
}

void PropertyTree::ensureVisible(PropertyRow* row, bool update, bool considerChildren)
{
	if(row == 0)
		return;
	if(row->isRoot())
		return;

	expandParents(row);

	PropertyRow* nonInlinedParent = row->findNonInlinedParent();
	Rect rowRect = nonInlinedParent->rect(this);
	if (considerChildren) {
		Rect childrenRect = nonInlinedParent->childrenRect(this);
		if (childrenRect.height() > 0)
			rowRect.h = childrenRect.bottom() - rowRect.top();
	}

	int topBorder = filterAreaHeight();
	int clientHeight = area_.height() - topBorder;
	if(offset_.y() > rowRect.top() - topBorder){
		offset_.setY(max(0, rowRect.top() - topBorder));
	}
	updateScrollBar();
	repaint();
}

void PropertyTree::onRowSelected(const std::vector<PropertyRow*>& rows, bool addSelection, bool adjustCursorPos)
{
	for (size_t i = 0; i < rows.size(); ++i) {
		PropertyRow* row = rows[i];
		if(!row->isRoot()) {
			bool addRowToSelection = !(addSelection && row->selected() && model()->selection().size() > 1) || i > 0;
			bool exclusiveSelection = !addSelection && i == 0;
			model()->selectRow(row, addRowToSelection, exclusiveSelection);
		}
	}
	if (!rows.empty()) {
		ensureVisible(rows.back(), true, false);
	}
	updateAttachedPropertyTree(false);
	onSelected();
}

bool PropertyTree::attach(const yasli::Serializers& serializers)
{
	bool changed = false;
	if (attached_.size() != serializers.size())
		changed = true;
	else {
		for (size_t i = 0; i < serializers.size(); ++i) {
			if (attached_[i].serializer() != serializers[i]) {
				changed = true;
				break;
			}
		}
	}

	// We can't perform plain copying here, as it was before:
	//   attached_ = serializers;
	// ...as move forwarder calls copying constructor with non-const argument
	// which invokes second templated constructor of Serializer, which is not what we need.
	if (changed) {
		attached_.assign(serializers.begin(), serializers.end());
		model_->clearUndo();
	}

	revertNoninterrupting();

	return changed;
}

void PropertyTree::attach(const yasli::Serializer& serializer)
{
	if (attached_.size() != 1 || attached_[0].serializer() != serializer) {
		attached_.clear();
		attached_.push_back(yasli::Object(serializer));
		model_->clearUndo();
	}
	revert();
}

void PropertyTree::attach(const yasli::Object& object)
{
	attached_.clear();
	attached_.push_back(object);

	revert();
}

void PropertyTree::detach()
{
	_cancelWidget();
	attached_.clear();
	model()->root()->clear();
	repaint();
}

void PropertyTree::_cancelWidget()
{
	// make sure that widget_ is null the moment widget is destroyed, so we
	// don't get focus callbacks to act on destroyed widget.
	std::auto_ptr<property_tree::InplaceWidget> widget(widget_.release());
}

int PropertyTree::revertObjects(vector<void*> objectAddresses)
{
	int result = 0;
	for (size_t i = 0; i < objectAddresses.size(); ++i) {
		if (revertObject(objectAddresses[i]))
			++result;
	}
	return result;
}

bool PropertyTree::revertObject(void* objectAddress)
{
	PropertyRow* row = model()->root()->findByAddress(objectAddress);
	if (row && row->isObject()) {
		// TODO:
		// revertObjectRow(row);
		return true;
	}
	return false;
}


void PropertyTree::revert()
{
	interruptDrag();
	_cancelWidget();
	capturedRow_ = 0;
	storePersistentFocusElement();

#ifdef PROFILE
	QElapsedTimer timer;
	timer.start();
#endif
	if (!attached_.empty()) {
		validatorBlock_->clear();


		PropertyOArchive oa(model_.get(), model_->root(), validatorBlock_.get());
		oa.setOutlineMode(outlineMode_);
		if (archiveContext_)
			oa.setLastContext(archiveContext_);
		oa.setFilter(config_.filter);

		Objects::iterator it = attached_.begin();
		onAboutToSerialize(oa);
		(*it)(oa);
		onSerialized(oa);
		PropertyTreeModel model2(this);
		while(++it != attached_.end()){
			PropertyOArchive oa2(&model2, model2.root(), validatorBlock_.get());
			oa2.setOutlineMode(outlineMode_);
			oa2.setLastContext(archiveContext_);
			yasli::Context treeContext(oa2, this);
			oa2.setFilter(config_.filter);
			onAboutToSerialize(oa2);
			(*it)(oa2);
			onSerialized(oa2);
			model_->root()->intersect(model2.root());
		}
		//revertTime_ = int(timer.elapsed());

		if (attached_.size() != 1)
			validatorBlock_->clear();
		applyValidation();
	}
	else
		model_->clear();
#ifdef PROFILE
	revertTime_ = int(timer.elapsed());
	printf("Revert time: %d\n", revertTime_);
#endif

	if (filterMode_) {
		if (model_->root())
			model_->root()->updateLabel(this, 0, false);
		resetFilter();
	}
	else {
		updateHeights();
	}

	restorePersistentFocusElement();
	repaint();
	updateAttachedPropertyTree(true);

	onReverted();
}

struct ValidatorVisitor
{
	ValidatorVisitor(ValidatorBlock* validator)
	: validator_(validator)
	{
	}

	ScanResult operator()(PropertyRow* row, PropertyTree* tree, int)
	{
		const void* rowHandle = row->searchHandle();
		int index = 0;
		int count = 0;
		if (validator_->findHandleEntries(&index, &count, rowHandle, row->searchType())) {
			validator_->markAsUsed(index, count);
			if (row->setValidatorEntry(index, count))
				row->setLabelChanged();
		}
		else {
			if (row->setValidatorEntry(0, 0))
				row->setLabelChanged();
		}

		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	ValidatorBlock* validator_;
};

void PropertyTree::applyValidation()
{
	if (!validatorBlock_->isEnabled())
		return;

	ValidatorVisitor visitor(validatorBlock_.get());
	model()->root()->scanChildren(visitor, this);

	int rootFirst = 0;
	int rootCount = 0;
	// Gather all the items with unknown handle/type pair at root level.
	validatorBlock_->mergeUnusedItemsWithRootItems(&rootFirst, &rootCount, model()->root()->searchHandle(), model()->root()->typeId());
	model()->root()->setValidatorEntry(rootFirst, rootCount);
	model()->root()->setLabelChanged();
}

void PropertyTree::revertNoninterrupting()
{
	if (!capturedRow_)
		revert();
}

void PropertyTree::apply(bool continuous)
{
#ifdef PROFILE
	QElapsedTimer timer;
	timer.start();
#endif
	if (!attached_.empty()) {
		Objects::iterator it;
		for(it = attached_.begin(); it != attached_.end(); ++it) {
			PropertyIArchive ia(model_.get(), model_->root());
			ia.setLastContext(archiveContext_);
			yasli::Context treeContext(ia, this);
 			ia.setFilter(config_.filter);
			onAboutToSerialize(ia);
			(*it)(ia);
			onSerialized(ia);
		}
	}
#ifdef PROFILE
	applyTime_ = int(timer.elapsed());
	printf("Apply time: %d\n", applyTime_);
#endif

	if (continuous)
		onContinuousChange();
	else
		onChanged();
}

void PropertyTree::applyInplaceEditor()
{
	if (widget_.get())
		widget_->commit();
}

bool PropertyTree::spawnWidget(PropertyRow* row, bool rename)
{
	if(!widget_.get() || widgetRow_ != row){
		interruptDrag();
		setWidget(0, 0);
		property_tree::InplaceWidget* newWidget = 0;
		if (!row->userReadOnly()) {
			if (((rename && row->userRenamable())) || (!rename && !row->userRenamable()))
				newWidget = row->createWidget(this);
		}
		setWidget(newWidget, row);
		return newWidget != 0;
	}
	return false;
}

void PropertyTree::addMenuHandler(PropertyRowMenuHandler* handler)
{
	menuHandlers_.push_back(handler);
}

void PropertyTree::clearMenuHandlers()
{
	for (size_t i = 0; i < menuHandlers_.size(); ++i)
	{
		PropertyRowMenuHandler* handler = menuHandlers_[i];
		delete handler;
	}
	menuHandlers_.clear();
}

static yasli::string quoteIfNeeded(const char* str)
{
	if (!str)
		return yasli::string();
	if (strchr(str, ' ') != 0) {
		yasli::string result;
		result = "\"";
		result += str;
		result += "\"";
		return result;
	}
	else {
		return yasli::string(str);
	}
}

bool PropertyTree::onContextMenu(PropertyRow* r, IMenu& menu)
{
	SharedPtr<PropertyRow> row(r);
	PropertyTreeMenuHandler* handler = new PropertyTreeMenuHandler();
	addMenuHandler(handler);
	handler->tree = this;
	handler->row = row;

	int num = row->count();
	for(int i = 0; i < num; ++i){
		PropertyRow* child = row->childByIndex(i);
		if(child->isContainer() && child->inlined())
			child->onContextMenu(menu, this);
	}
	row->onContextMenu(menu, this);
	if(config_.undoEnabled){
		if(!menu.isEmpty())
			menu.addSeparator();
		menu.addAction("Undo", "Ctrl+Z", model()->canUndo() ? 0 : MENU_DISABLED, handler, &PropertyTreeMenuHandler::onMenuUndo);
	}
	if(!menu.isEmpty())
		menu.addSeparator();

	menu.addAction("Copy", "Ctrl+C", 0, handler, &PropertyTreeMenuHandler::onMenuCopy);

	if(!row->userReadOnly()){
		menu.addAction("Paste", "Ctrl+V", canBePasted(row) ? 0 : MENU_DISABLED, handler, &PropertyTreeMenuHandler::onMenuPaste);
	}

	if (model()->root() && !model()->root()->userReadOnly()) {
		PropertyTreeMenuHandler* rootHandler = new PropertyTreeMenuHandler();
		rootHandler->tree = this;
		rootHandler->row = model()->root();
		menu.addSeparator();
		menu.addAction("Copy All", "", 0, rootHandler, &PropertyTreeMenuHandler::onMenuCopy);
		menu.addAction("Paste All", "", canBePasted(model()->root()) ? 0 : MENU_DISABLED, rootHandler, &PropertyTreeMenuHandler::onMenuPaste);
		addMenuHandler(rootHandler);
	}
	menu.addSeparator();

	menu.addAction("Filter...", "Ctrl+F", 0, handler, &PropertyTreeMenuHandler::onMenuFilter);
	IMenu* filter = menu.addMenu("Filter by");
	{
		yasli::string nameFilter = "#";
		nameFilter += quoteIfNeeded(row->labelUndecorated());
		handler->filterName = nameFilter;
		filter->addAction((yasli::string("Name:\t") + nameFilter).c_str(), 0, handler, &PropertyTreeMenuHandler::onMenuFilterByName);

		yasli::string valueFilter = "=";
		valueFilter += quoteIfNeeded(row->valueAsString().c_str());
		handler->filterValue = valueFilter;
		filter->addAction((yasli::string("Value:\t") + valueFilter).c_str(), 0, handler, &PropertyTreeMenuHandler::onMenuFilterByValue);

		yasli::string typeFilter = ":";
        typeFilter += quoteIfNeeded(row->typeNameForFilter(this).c_str());
		handler->filterType = typeFilter;
		filter->addAction((yasli::string("Type:\t") + typeFilter).c_str(), 0, handler, &PropertyTreeMenuHandler::onMenuFilterByType);
	}

	// menu.addSeparator();
	// menu.addAction(TRANSLATE("Decompose"), row).connect(this, &PropertyTree::onRowMenuDecompose);
	return true;
}

void PropertyTree::onRowMouseMove(PropertyRow* row, Point point)
{
	PropertyDragEvent e;
	e.tree = this;
	e.pos = point;
	e.start = pressPoint_;
	e.totalDelta = pressDelta_;
	row->onMouseDrag(e);
	repaint();
}

struct DecomposeProxy
{
	DecomposeProxy(SharedPtr<PropertyRow>& row) : row(row) {}
	
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar)
	{
		ar(row, "row", "Row");
	}

	SharedPtr<PropertyRow>& row;
};

void PropertyTree::onRowMenuDecompose(PropertyRow* row)
{
  // SharedPtr<PropertyRow> clonedRow = row->clone();
  // DecomposeProxy proxy(clonedRow);
  // edit(Serializer(proxy), 0, IMMEDIATE_UPDATE, this);
}

void PropertyTree::onModelUpdated(const PropertyRows& rows, bool needApply)
{
	_cancelWidget();

	if(config_.immediateUpdate){
		if (needApply)
			apply();

		if(autoRevert_)
			revert();
		else {
			updateHeights();
			updateAttachedPropertyTree(true);
		}
	}
	else {
		repaint();
	}
}

void PropertyTree::onModelPushUndo(PropertyTreeOperator* op, bool* handled)
{
	onPushUndo();
}

void PropertyTree::setWidget(property_tree::InplaceWidget* widget, PropertyRow* widgetRow)
{
	_cancelWidget();
	widget_.reset(widget);
	widgetRow_ = widgetRow;
	model()->dismissUpdate();
	if(widget_.get()){
		YASLI_ASSERT(widget_->actualWidget());
		_arrangeChildren();
		widget_->showPopup();
		repaint();
	}
}

void PropertyTree::setExpandLevels(int levels)
{
	config_.expandLevels = levels;
    model()->setExpandLevels(levels);
}

PropertyRow* PropertyTree::selectedRow()
{
    const PropertyTreeModel::Selection &sel = model()->selection();
    if(sel.empty())
        return 0;
    return model()->rowFromPath(sel.front());
}

bool PropertyTree::getSelectedObject(yasli::Object* object)
{
	const PropertyTreeModel::Selection &sel = model()->selection();
	if(sel.empty())
		return 0;
	PropertyRow* row = model()->rowFromPath(sel.front());
	while (row && !row->isObject())
		row = row->parent();
	if (!row)
		return false;

	if (row->isObject()) {
		if (PropertyRowObject* obj = static_cast<PropertyRowObject*>(row)) {
			*object = obj->object();
			return true;
		}
	}
	return false;
}

bool PropertyTree::setSelectedRow(PropertyRow* row)
{
	TreeSelection sel;
	if(row)
		sel.push_back(model()->pathFromRow(row));
	if (model()->selection() != sel) {
		model()->setSelection(sel);
		if (row)
			ensureVisible(row);
		updateAttachedPropertyTree(false);
		repaint();
		return true;
	}
	return false;
}

int PropertyTree::selectedRowCount() const
{
	return (int)model()->selection().size();
}

PropertyRow* PropertyTree::selectedRowByIndex(int index)
{
	const PropertyTreeModel::Selection &sel = model()->selection();
	if (size_t(index) >= sel.size())
		return 0;

	return model()->rowFromPath(sel[index]);
}


bool PropertyTree::selectByAddress(const void* addr, bool keepSelectionIfChildSelected)
{
	return selectByAddresses(vector<const void*>(1, addr), keepSelectionIfChildSelected);
}

bool PropertyTree::selectByAddresses(const vector<const void*>& addresses, bool keepSelectionIfChildSelected)
{
	return selectByAddresses(&addresses.front(), addresses.size(), keepSelectionIfChildSelected);
}



bool PropertyTree::selectByAddresses(const void* const* addresses, size_t addressCount, bool keepSelectionIfChildSelected)
{
	bool result = false;
	if (model()->root()) {
		bool keepSelection = false;
		vector<PropertyRow*> rows;
		for (size_t i = 0; i < addressCount; ++i) {
			const void* addr = addresses[i];
			PropertyRow* row = model()->root()->findByAddress(addr);

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

			if (row)
				rows.push_back(row);
		}

		if (!keepSelection) {
			TreeSelection sel;
			for (size_t j = 0; j < rows.size(); ++j) {
				PropertyRow* row = rows[j];
				if(row) {
					sel.push_back(model()->pathFromRow(row));
					ensureVisible(row, true, false);
				}
			}
		}
	}
	return result;
}

void PropertyTree::setUndoEnabled(bool enabled, bool full)
{
	config_.undoEnabled = enabled;
	config_.fullUndo = full;
    model()->setUndoEnabled(enabled);
    model()->setFullUndo(full);
}

void PropertyTree::attachPropertyTree(PropertyTree* propertyTree) 
{ 
	// TODO:
	// if(attachedPropertyTree_)
	// 	disconnect(attachedPropertyTree_, SIGNAL(signalChanged()), this, SLOT(onAttachedTreeChanged()));
	attachedPropertyTree_ = propertyTree; 
	//if (attachedPropertyTree_)
	//	connect(attachedPropertyTree_, SIGNAL(signalChanged()), this, SLOT(onAttachedTreeChanged()));
	updateAttachedPropertyTree(true); 
}

void PropertyTree::detachPropertyTree()
{
	attachPropertyTree(0);
}

void PropertyTree::setAutoHideAttachedPropertyTree(bool autoHide)
{
	autoHideAttachedPropertyTree_ = autoHide;
}

void PropertyTree::getSelectionSerializers(yasli::Serializers* serializers)
{
	TreeSelection::const_iterator i;
	for(i = model()->selection().begin(); i != model()->selection().end(); ++i){
		PropertyRow* row = model()->rowFromPath(*i);
		if (!row)
			continue;


		while(row && ((row->inlined() || row->inlinedBefore()) || row->isLeaf())) {
			row = row->parent();
		}
		if (outlineMode_) {
			PropertyRow* topmostContainerElement = 0;
			PropertyRow* r = row;
			while (r && r->parent()) {
				if (r->parent()->isContainer())
					topmostContainerElement = r;
				r = r->parent();
			}
			if (topmostContainerElement != 0)
				row = topmostContainerElement;
		}
		Serializer ser = row->serializer();

		if (ser)
			serializers->push_back(ser);
	}
}

void PropertyTree::updateAttachedPropertyTree(bool revert)
{
	if(attachedPropertyTree_) {
 		Serializers serializers;
 		getSelectionSerializers(&serializers);
 		if (!attachedPropertyTree_->attach(serializers) && revert)
			attachedPropertyTree_->revert();
 	}
}


void PropertyTree::RowFilter::parse(const char* filter)
{
	for (int i = 0; i < NUM_TYPES; ++i) {
		start[i].clear();
		substrings[i].clear();
		tillEnd[i] = false;
	}

	YASLI_ESCAPE(filter != 0, return);

	vector<char> filterBuf(filter, filter + strlen(filter) + 1);
	for (size_t i = 0; i < filterBuf.size(); ++i)
		filterBuf[i] = tolower(filterBuf[i]);

	const char* str = &filterBuf[0];

	Type type = NAME_VALUE;
	while (true)
	{
		bool fromStart = false;
		while (*str == '^') {
			fromStart = true;
			++str;
		}

		const char* tokenStart = str;
		
		if (*str == '\"')
		{
			++str;
			while(*str != '\0' && *str != '\"')
				++str;
		}
		else
		{
			while (*str != '\0' && *str != ' ' && *str != '*' && *str != '=' && *str != ':' && *str != '#')
					++str;
		}
		if (str != tokenStart) {
			if (*tokenStart == '\"' && *str == '\"') {
				start[type].assign(tokenStart + 1, str);
				tillEnd[type] = true;
				++str;
			}
			else
			{
				if (fromStart)
					start[type].assign(tokenStart, str);
				else
					substrings[type].push_back(yasli::string(tokenStart, str));
			}
		}
		while (*str == ' ')
			++str;
		if (*str == '#') {
			type = NAME;
			++str;
		}
		else if (*str == '=') {
			type = VALUE;
			++str;
		}
		else if(*str == ':') {
			type = TYPE;
			++str;
		}
		else if (*str == '\0')
			break;
	}
}

bool PropertyTree::RowFilter::match(const char* textOriginal, Type type, size_t* matchStart, size_t* matchEnd) const
{
	YASLI_ESCAPE(textOriginal, return false);

	char* text;
	{
		size_t textLen = strlen(textOriginal);
        text = (char*)alloca((textLen + 1));
		memcpy(text, textOriginal, (textLen + 1));
		for (char* p = text; *p; ++p)
			*p = tolower(*p);
	}
	
	const yasli::string &start = this->start[type];
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

	const vector<yasli::string> &substrings = this->substrings[type];

	const char* startPos = text;

	if (matchStart)
		*matchStart = 0;
	if (matchEnd)
		*matchEnd = 0;
	if (!start.empty()) {
		if (strncmp(text, start.c_str(), start.size()) != 0){
            //_freea(text);
			return false;
		}
		if (matchEnd)
			*matchEnd = start.size();
		startPos += start.size();
	}

	size_t numSubstrings = substrings.size();
	for (size_t i = 0; i < numSubstrings; ++i) {
		const char* substr = strstr(startPos, substrings[i].c_str());
		if (!substr){
			return false;
		}
		startPos += substrings[i].size();
		if (matchStart && i == 0 && start.empty()) {
			*matchStart = substr - text;
		}
		if (matchEnd)
			*matchEnd = substr - text + substrings[i].size();
	}
	return true;
}

PropertyRow* PropertyTree::rowByPoint(const Point& pointInWidgetSpace)
{
	Point pt = pointToRootSpace(pointInWidgetSpace);
	const Layout& l = *layout_;
	int count = (int)l.rectangles.size();
	for (int i = (int)l.rectangles.size() - 1; i >= 0; --i)
		if (l.rectangles[i].contains(pt))
			return l.rows[i];
	return model()->root();
}

Point PropertyTree::pointToRootSpace(const Point& point) const
{
	return Point(point.x() + offset_.x(), point.y() + offset_.y());
}

Point PropertyTree::pointFromRootSpace(const Point& point) const
{
	return Point(point.x() - offset_.x() + area_.left(), point.y() - offset_.y() + area_.top());
}

bool PropertyTree::toggleRow(PropertyRow* row)
{
	if(!row->canBeToggled(this))
		return false;
	expandRow(row, !row->expanded());
	return true;
}

bool PropertyTree::_isCapturedRow(const PropertyRow* row) const
{
	return capturedRow_ == row;
}

void PropertyTree::setValueColumnWidth(float valueColumnWidth) 
{
	if (style_->valueColumnWidth != valueColumnWidth)
	{
		style_->valueColumnWidth = valueColumnWidth; 
		updateHeights(false);
		repaint();
	}
}

void PropertyTree::setArchiveContext(yasli::Context* lastContext)
{
	archiveContext_ = lastContext;
}

PropertyTree::PropertyTree(const PropertyTree&)
{
}


PropertyTree& PropertyTree::operator=(const PropertyTree&)
{
	return *this;
}

void PropertyTree::onAttachedTreeChanged()
{
	revert();
}

Point PropertyTree::_toWidget(Point point) const
{
	return Point(point.x() - offset_.x(), point.y() - offset_.y());
}

struct ValidatorIconVisitor
{
	ScanResult operator()(PropertyRow* row, PropertyTree* tree, int)
	{
		row->resetValidatorIcons();
		if (row->validatorCount()) {
			bool hasErrors = false;
			bool hasWarnings = false;
			if (const ValidatorEntry* validatorEntries = tree->_validatorBlock()->getEntry(row->validatorIndex(), row->validatorCount())) {
				for (int i = 0; i < row->validatorCount(); ++i) {
					const ValidatorEntry* validatorEntry = validatorEntries + i;
					if (validatorEntry->type == VALIDATOR_ENTRY_ERROR)
						hasErrors = true;
					else if (validatorEntry->type == VALIDATOR_ENTRY_WARNING)
						hasWarnings = true;
				}
			}

			if (hasErrors || hasWarnings)
			{
				PropertyRow* lastClosedParent = 0;
				PropertyRow* current = row->parent();
				bool lastWasPulled = row->inlined() || row->inlinedBefore();
				while (current && current->parent()) {
					if (!current->expanded() && !lastWasPulled && current->visible(tree))
						lastClosedParent = current;
					lastWasPulled = current->inlined() || current->inlinedBefore();
					current = current->parent();
				}
				if (lastClosedParent)
					lastClosedParent->addValidatorIcons(hasWarnings, hasErrors);
			}
		}
		return SCAN_CHILDREN_SIBLINGS;
	}
};

void PropertyTree::updateValidatorIcons()
{
	if (!validatorBlock_->isEnabled())
		return;
	ValidatorIconVisitor op;
	model()->root()->scanChildren(op, this);
	model()->root()->setLabelChangedToChildren();
}

void PropertyTree::setDefaultTreeStyle(const PropertyTreeStyle& treeStyle)
{
	defaultTreeStyle_ = treeStyle;
}

void PropertyTree::setTreeStyle(const PropertyTreeStyle& style)
{
	*style_ = style;
	updateHeights(true);
}

void PropertyTree::setPackCheckboxes(bool pack)
{
	style_->packCheckboxes = pack;
	updateHeights(true);
}

bool PropertyTree::packCheckboxes() const
{
	return style_->packCheckboxes;
}

void PropertyTree::setCompact(bool compact)
{
	style_->compact = compact;
	repaint();
}

bool PropertyTree::compact() const
{
	return style_->compact;
}

void PropertyTree::setRowSpacing(float rowSpacing)
{
	style_->rowSpacing = rowSpacing;
}

float PropertyTree::rowSpacing() const
{
	return style_->rowSpacing;
}

float PropertyTree::valueColumnWidth() const
{
	return style_->valueColumnWidth;
}

void PropertyTree::setFullRowMode(bool fullRowMode)
{
	style_->fullRowMode = fullRowMode;
	repaint();
}

bool PropertyTree::fullRowMode() const
{
	return style_->fullRowMode;
}

bool PropertyTree::containsErrors() const
{
	return validatorBlock_->containsErrors();
}

void PropertyTree::focusFirstError()
{
	jumpToNextHiddenValidatorIssue(true, model()->root());
}

void PropertyTree::drawLayout(property_tree::IDrawContext& context, int h)
{
	int numElements = layout_->rectangles.size();
	for (size_t i = 0; i < numElements; ++i) {
		if (findInSortedVector(hiddenLayoutElements_, (int)i))
			continue;
		const Rect& rect = layout_->rectangles[i];
		if (rect.bottom() - offset_.y() < 0)
			continue;
		if (rect.top() - offset_.y() > h)
			continue;
		const LayoutElement& e = layout_->elements[i];
		if (e.rowPart != PART_ROW_AREA)
			continue;
		PropertyRow* row = layout_->rows[i];
		if (!row)
			continue;
		row->drawElement(context, (property_tree::RowPart)e.rowPart, rect, e.rowPartSubindex);
	}
	for (size_t i = 0; i < numElements; ++i) {
		if (findInSortedVector(hiddenLayoutElements_, (int)i))
			continue;
		const Rect& rect = layout_->rectangles[i];
		if (rect.bottom() - offset_.y() < 0)
			continue;
		if (rect.top() - offset_.y() > h)
			continue;
		const LayoutElement& e = layout_->elements[i];
		if (e.rowPart == PART_ROW_AREA)
			continue;
		PropertyRow* row = layout_->rows[i];
		if (!row)
			continue;
		row->drawElement(context, (property_tree::RowPart)e.rowPart, rect, e.rowPartSubindex);
	}
}

static int findLayoutElementByFocusIndex(Layout& layout,
	int* currentX, int* currentY,
	int element, int x, int y)
{
	int startX = *currentX;
	int startY = *currentY;
	const LayoutElement& e = layout.elements[element];
	if (e.childrenList >= 0) {
		const vector<int>& children = layout.childrenLists[e.childrenList].children;
		for (size_t i = 0; i < children.size(); ++i) {
			int childrenElement = children[i];
			if (*currentX == x && *currentY == y)
				return childrenElement;
			if (e.type == HORIZONTAL)
				++*currentX;
			else if (e.type == VERTICAL)
				++*currentY;
			int childrenResult = findLayoutElementByFocusIndex(layout, &*currentX, &*currentY, childrenElement, x, y);
			if (childrenResult != -1)
				return childrenResult;
		}
	}
	*currentX = startX;
	*currentY = startY;
	return -1;
}

int PropertyTree::layoutElementByFocusIndex(int x, int y)
{
	int currentX = 0;
	int currentY = 0;
	int rootElement = 0;
	return findLayoutElementByFocusIndex(*layout_, &currentX, &currentY, rootElement, x, y);
}

static void findLayoutChildren(vector<int>* elements, const property_tree::Layout& layout, int element)
{
	elements->push_back(element);
	const LayoutElement& e = layout.elements[element];
	if (e.childrenList >= 0) {
		const vector<int>& children = layout.childrenLists[e.childrenList].children;
		for (size_t i = 0; i < children.size(); ++i) {
			findLayoutChildren(elements, layout, children[i]);
		}
	}
}

void PropertyTree::drawRowLayout(property_tree::IDrawContext& context, PropertyRow* row)
{
	int elementIndex = row->layoutElement();

	vector<int> elements;
	findLayoutChildren(&elements, *layout_, elementIndex);

	for (size_t j = 0; j < elements.size(); ++j) {
		int i = elements[j];
		const Rect& rect = layout_->rectangles[i];
		const LayoutElement& e = layout_->elements[i];
		if (e.rowPart != PART_ROW_AREA)
			continue;
		PropertyRow* row = layout_->rows[i];
		if (!row)
			continue;
		row->drawElement(context, (property_tree::RowPart)e.rowPart, rect, e.rowPartSubindex);
	}

	for (size_t j = 0; j < elements.size(); ++j) {
		int i = elements[j];
		const Rect& rect = layout_->rectangles[i];
		const LayoutElement& e = layout_->elements[i];
		if (e.rowPart == PART_ROW_AREA)
			continue;
		PropertyRow* row = layout_->rows[i];
		if (!row)
			continue;
		row->drawElement(context, (property_tree::RowPart)e.rowPart, rect, e.rowPartSubindex);
	}
}

void PropertyTree::setDraggedRow(PropertyRow* row)
{
	hiddenLayoutElements_.clear();
	if (row && row->layoutElement() >= 0) {
		findLayoutChildren(&hiddenLayoutElements_, *layout_, row->layoutElement());
		std::sort(hiddenLayoutElements_.begin(), hiddenLayoutElements_.end());
	}
}

void PropertyTree::storePersistentFocusElement()
{
	LayoutElement* element = 0;
	if (size_t(focusedLayoutElement_) < layout_->elements.size()) {
		element = &layout_->elements[focusedLayoutElement_];
	}
	if (element) {
		PersistentLayoutElement& pe = *persistentFocusedLayoutElement_;
		PropertyRow* row = layout_->rows[focusedLayoutElement_];
		pe.row = model()->pathFromRow(row);
		pe.part = (RowPart)element->rowPart;
		pe.partIndex = element->rowPartSubindex;
	}

}

void PropertyTree::restorePersistentFocusElement()
{
	const PersistentLayoutElement& pe = *persistentFocusedLayoutElement_;
	PropertyRow* row = model()->rowFromPath(pe.row);
	focusedLayoutElement_ = findRowElement(*layout_, row, pe.part, pe.partIndex);
	if (focusedLayoutElement_ == 0 && !pe.row.empty()) {
		while (row) {
			focusedLayoutElement_ = findFocusableRowElement(*layout_, row);
            if (focusedLayoutElement_ != 0)
                break;
			row = row->parent();
		}
	}
	if (row)
		model()->selectRow(row, true);
}

PropertyRow* PropertyTree::focusedRow() const
{
	if (focusedLayoutElement_ == 0)
		return 0;
	if (size_t(focusedLayoutElement_) >= layout_->rows.size())
		return 0;
	return layout_->rows[focusedLayoutElement_];
}

// vim:ts=4 sw=4:
