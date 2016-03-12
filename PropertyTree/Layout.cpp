/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2016 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
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

namespace property_tree {

static bool isFullRow(const PropertyRow* row, const PropertyTree* tree)
{
    if (tree->fullRowMode())
		return true;
	if (row->parent() && row->parent()->isContainer() && tree->fullRowContainers())
		return true;
	return row->userFullRow();
}

static void populateRowArea(bool* hasNonInlinedChildren, Layout* l, int rowArea, PropertyRow* row, PropertyTree* tree, int indexForContainerElement, bool isInlined)
{
	PropertyRow::WidgetPlacement placement = row->widgetPlacement();
	int widgetSizeMin = row->widgetSizeMin(tree);
	int labelMin = row->textSizeInitial();
	char labelBuffer[16] = ""; 
	const char* label = row->rowText(labelBuffer, tree, indexForContainerElement);
	ElementType labelElementType = (isFullRow(row, tree) || row->inlined()) ? FIXED_SIZE : EXPANDING;
	int labelPriority = 1;
	int labelElement = -1;
	int widgetElement = -1;

	// for container elements we put array index text before inlined widgets
	bool labelBeforeInlined = false;
	if (row->parent() && row->parent()->isContainer() && 
		(placement == PropertyRow::WIDGET_VALUE || placement == PropertyRow::WIDGET_NONE)) {
		labelBeforeInlined = true;
		labelPriority = 2;
		if (label[0])
			labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, NOT_FOCUSABLE);
	}

	int count = (int)row->count();
	bool hasInlinedChildren = false;
	for (size_t j = 0; j < count; ++j) {
		PropertyRow* child = row->childByIndex(j);
		if (!child->visible(tree))
			continue;
		if (child->inlinedBefore()) {
			populateRowArea(hasNonInlinedChildren, l, rowArea, child, tree, 0, true);
			hasInlinedChildren = true;
		}
		if (child->inlined()) {
			hasInlinedChildren = true;
		}
	}
	if (labelElementType == EXPANDING && (hasInlinedChildren || placement == PropertyRow::WIDGET_VALUE)) {
		labelElementType = EXPANDING_MAGNET;
	}

	const FocusFlags widgetFocusFlags = row->isSelectable() ? FOCUSABLE : NOT_FOCUSABLE;
	switch (placement) {
	case PropertyRow::WIDGET_ICON:
	widgetElement = l->addElement(rowArea, FIXED_SIZE, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusFlags);
	if (label[0])
		l->addElement(rowArea, isInlined ? FIXED_SIZE : EXPANDING, row, PART_LABEL, labelMin, 0, labelPriority, NOT_FOCUSABLE);
	break;
	case PropertyRow::WIDGET_VALUE: {
		if (!labelBeforeInlined && label[0])
			labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, NOT_FOCUSABLE);
		ElementType widgetElementType = row->userFullRow() ? EXPANDING :
		row->userFixedWidget() ? FIXED_SIZE : EXPANDING;
		widgetElement = l->addElement(rowArea, widgetElementType, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusFlags);
	}
	break;
	case PropertyRow::WIDGET_NONE:
	if (!labelBeforeInlined && label[0])
		labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, NOT_FOCUSABLE);
	break;
	case PropertyRow::WIDGET_AFTER_NAME:
	if (label[0])
		labelElement = l->addElement(rowArea, FIXED_SIZE, row, PART_LABEL, labelMin, 0, labelPriority, NOT_FOCUSABLE);
	widgetElement = l->addElement(rowArea, FIXED_SIZE, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusFlags);
	break;
	case PropertyRow::WIDGET_AFTER_INLINED: {
		if (label[0])
			labelElement = l->addElement(rowArea, labelElementType, row, PART_LABEL, labelMin, 0, labelPriority, NOT_FOCUSABLE);
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
			populateRowArea(hasNonInlinedChildren, l, rowArea, child, tree, 0, true);
		}
		else if (!child->inlinedBefore())
			*hasNonInlinedChildren = true;
	}

	if (placement == PropertyRow::WIDGET_AFTER_INLINED) {
		widgetElement = l->addElement(rowArea, FIXED_SIZE, row, PART_WIDGET, widgetSizeMin, 0, 0, widgetFocusFlags);
	}
	row->setLayoutElement(rowArea);
	if (labelElement > 0) {
		if (*hasNonInlinedChildren || (row->parent() && row->parent()->isContainer())) {
			l->elements[labelElement].focusFlags = FOCUSABLE;
		} else if( hasInlinedChildren ) {
			l->elements[labelElement].focusFlags = FORWARDS_FOCUS;
		}
	}

	// add icons that can be used to jump to the nested warning/error
	if ( row->validatorHasWarnings() )
		l->addElement(rowArea, FIXED_SIZE, row, PART_VALIDATOR_WARNING_ICON, tree->_defaultRowHeight(), 0, 0, FOCUSABLE);
	if ( row->validatorHasErrors() )
		l->addElement(rowArea, FIXED_SIZE, row, PART_VALIDATOR_ERROR_ICON, tree->_defaultRowHeight(), 0, 0, FOCUSABLE);
}


void addValidatorsToLayout_r(PropertyTree* tree, Layout* l, int parentElement, PropertyRow* row)
{
	if (row->validatorCount()) {
		if (const ValidatorEntry* validatorEntries = tree->_validatorBlock()->getEntry(row->validatorIndex(), row->validatorCount())) {
			for (int i = 0; i < row->validatorCount(); ++i) {
				const ValidatorEntry* validatorEntry = validatorEntries + i;
				l->addElement(parentElement, HEIGHT_BY_WIDTH, row, PART_VALIDATOR, 40, 40, 0, NOT_FOCUSABLE, i);
			}
		}
	}

	// put bubbles of inlined properties below the property itself
	int numChildren = row->count();
	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* child = row->childByIndex(i);
		if (!child)
			continue;
		if (child->inlined() || child->inlinedBefore()) {
			addValidatorsToLayout_r(tree, l, parentElement, child);
		}
	}
}

static int rangeOfPackableRows(PropertyRow* parentRow, int index)
{
	int result = 0;
	int count = parentRow->count();
	for (int i = index; i < count; ++i) {
		PropertyRow* child = parentRow->childByIndex(i);
		if (!child->inlined() && !child->inlinedBefore() && child->canBePacked()) {
			result += 1;
		} else {
			return result;
		}
	}
	return result;
}

void populateChildrenArea_r(Layout* l, int parentElement, PropertyRow* parentRow, PropertyTree* tree, int indentationLevel)
{
	int rowHeight = int(tree->_defaultRowHeight() * max(0.1f, tree->treeStyle().rowSpacing) + 0.5f);

	// can alternate with multi-column area for packed checkboxes
	int currentChildrenArea = parentElement;
	const bool packCheckboxesAtThisLevel = tree->treeStyle().packCheckboxes && !parentRow->userPackCheckboxes();
	int packRowsEnd = -1;

	// assuming that parentRow is expanded
	int count = (int)parentRow->count();
	for (int i = 0 ; i < count; ++i) {
		PropertyRow* child = parentRow->childByIndex(i);
		if (!child->visible(tree))
			continue;

		// children of inlined elements here
		if (child->inlined() || child->inlinedBefore()) {
			populateChildrenArea_r(l, parentElement, child, tree, indentationLevel);
			continue;
		}
		
		// begin packing sequence of packable rows (usually checkboxes)
		if (packCheckboxesAtThisLevel && i > packRowsEnd) {
			int rangeLength = rangeOfPackableRows(parentRow, i);
			const int minCheckboxesToPack = 6;
			if (rangeLength >= minCheckboxesToPack) {
				packRowsEnd = i + rangeLength - 1;
				currentChildrenArea = l->addElement(parentElement, MULTI_COLUMN, parentRow, PART_CHILDREN_AREA, 0, 0, 0, NOT_FOCUSABLE);
			}
		}

		int rowArea = l->addElement(currentChildrenArea, HORIZONTAL, child, PART_ROW_AREA, 0, rowHeight, 0, NOT_FOCUSABLE);
		bool showPlus = !(tree->treeStyle().compact && parentRow->isRoot());
		if (showPlus)
			l->addElement(rowArea, FIXED_SIZE, child, PART_PLUS, rowHeight, 0, 0, NOT_FOCUSABLE);

		bool hasNonInlinedChildren = false;
		populateRowArea(&hasNonInlinedChildren, l, rowArea, child, tree, i, false);

		// add validator bubbles from the row itself and its inlined chlildren
		addValidatorsToLayout_r(tree, l, currentChildrenArea, child);

		if (child->expanded()) {
			int indentationAndContent = l->addElement(currentChildrenArea, HORIZONTAL, child, PART_INDENTATION_AND_CONTENT_AREA, 0, 0, 0, NOT_FOCUSABLE);
			if (showPlus) {
				int indentation;
				if (indentationLevel == 0 || (tree->treeStyle().compact && indentationLevel == 1)) {
					indentation = int(rowHeight * tree->treeStyle().firstLevelIndent + 0.5f);
				} else {
					indentation = int(rowHeight * tree->treeStyle().levelIndent + 0.5f);
				}
				l->addElement(indentationAndContent, FIXED_SIZE, child, PART_INDENTATION, indentation, 0, 0, NOT_FOCUSABLE);
			}
			int contentArea = l->addElement(indentationAndContent, child->userPackCheckboxes() ? MULTI_COLUMN : VERTICAL, child, PART_CHILDREN_AREA, 0, 0, 0, NOT_FOCUSABLE);
			populateChildrenArea_r(l, contentArea, child, tree, indentationLevel + 1);
		}

		if (i == packRowsEnd) {
			// end sequence of packed (multi-column) rows and return to normal layout
			currentChildrenArea = parentElement;
		}
	}
}

static void restoreColumns(LayoutColumn (*columns)[MAX_COLUMNS], int* numColumns, const Layout &l, int element) {
	const LayoutElement& e = l.elements[element];
	const vector<int>& children = l.childrenLists[e.childrenList].children;
	int count = (int)children.size();
	int currentColumn = 0;
	for (int i = 0; i < count; ++i) {
		const LayoutElement& child = l.elements[children[i]];
		if (child.beginsColumn) {
			(*columns)[currentColumn].end = i;
			if (currentColumn + 1< MAX_COLUMNS) {
				++currentColumn;
				(*columns)[currentColumn].start = i;
			}
		}
	}
	(*columns)[currentColumn].end = count;
	*numColumns = currentColumn+1;
}

static void calculateColumns(LayoutColumn (*columns)[MAX_COLUMNS], int* numColumns, int* maxColumnLength,
							 Layout * l, int element, int widthAvailable)
{
	const LayoutElement& e = l->elements[element];
	const vector<int>& children = l->childrenLists[e.childrenList].children;
	int count = (int)children.size();

	// access width, in vertical pass
	int minimalWidth = max(1, l->minimalWidths[element]);
	*numColumns = clamp(widthAvailable / minimalWidth, 1, MAX_COLUMNS);
	int columnWidth = widthAvailable / *numColumns;
	int length = 0;
	for (int i = 0; i < count; ++i) {
		int childIndex = children[i];
		const LayoutElement& child = l->elements[childIndex];
		int childFixedLength = e.type == HORIZONTAL ? l->minimalWidths[childIndex] : l->minimalHeights[childIndex];
		length += childFixedLength;
	}
	int lastColumn = 0;
	for (int i = 0; i < *numColumns - 1; ++i) {
		LayoutColumn& column = (*columns)[i];
		column.left = lastColumn;
		column.width = columnWidth;
		lastColumn = lastColumn + columnWidth;
	}
	(*columns)[*numColumns-1].left = lastColumn;
	(*columns)[*numColumns-1].width = widthAvailable - lastColumn;

	int columnLength = 0;
	int desiredLength = length / *numColumns;
	int currentColumn = 0;
	for (int i = 0; i < count; ++i) {
		int childIndex = children[i];
		LayoutElement& child = l->elements[childIndex];
		int childFixedLength = e.type == HORIZONTAL ? l->minimalWidths[childIndex] : l->minimalHeights[childIndex];
		if (columnLength + childFixedLength > desiredLength && currentColumn + 1 < *numColumns) {
			child.beginsColumn = true;
			(*columns)[currentColumn].end = i;
			++currentColumn;
			(*columns)[currentColumn].start = i;
			*maxColumnLength = max(*maxColumnLength, columnLength);
			columnLength = childFixedLength;
		} else {
			columnLength += childFixedLength;
		}
	}
	(*columns)[currentColumn].end = count;
	*maxColumnLength = max(*maxColumnLength, columnLength);
}

void calculateMinimalSizes_r(int* outMinSize, ElementType orientation, Layout* l, int element )
{
	int minSize = 0;
	// Width and height are required to define minimal size of HEIGHT_BY_WIDTH elements.
	LayoutElement& e = l->elements[element];
	if (e.type == FIXED_SIZE ||
		e.type == EXPANDING ||
		e.type == EXPANDING_MAGNET ||
		e.type == HEIGHT_BY_WIDTH) {
		if ( orientation == HORIZONTAL ) {
			minSize = e.minWidth;
		} else {
			if (e.type == HEIGHT_BY_WIDTH) {
				minSize = l->heightByWidth(l->heightByWidthArgument, element, e.rowPartSubindex, l->rectangles[element].w);
			} else {
				minSize = e.minHeight;
			}
		}
	} else {
		int s = 0;
		if (e.childrenList != -1) {
			const vector<int>& children = l->childrenLists[e.childrenList].children;
			int count = (int)children.size();
			for (int i = 0; i < count; ++i) {
				int childrenSize = 0;;
				LayoutElement& child = l->elements[children[i]];
				calculateMinimalSizes_r(&childrenSize, orientation, l, children[i]);
				if (e.type == HORIZONTAL) {
					if ( orientation == HORIZONTAL ) {
						// FIXME: priorities
						s += childrenSize;
					} else {
						s = max(s, childrenSize);
					}
				} else if (e.type == VERTICAL || e.type == MULTI_COLUMN) {
					if ( orientation == VERTICAL ) {
						if (child.priority == 0)
							s += childrenSize;
					} else {
						// Exception for vertical layouts, so only individual rows
						// are getting outside of window, when it is too narrow.
						if (e.type != VERTICAL)  {
							s = max(s, childrenSize);
						}
					}
				}
			}

			if (e.type == MULTI_COLUMN && orientation == VERTICAL) {
				LayoutColumn columns[MAX_COLUMNS] = {
					{ 0, int(children.size()), 0, 0 }
				};
				int numColumns = 1;
				int maxColumnLength = 0;
				const Rect& rect = l->rectangles[element];
				calculateColumns(&columns, &numColumns, &maxColumnLength, &*l, element, rect.w);
				// repeat horizontal layout pass knowing that elements
				// are split in the columns now
				for (int i = 0; i < count; ++i) {
					int childIndex = children[i];
					int column = 0; 
					for (int j = 0; j < numColumns; ++j) {
						if (i >= columns[j].start) {
							column = j;
						}
					}
					calculateRectangles_r(&*l, HORIZONTAL, childIndex, columns[column].width, columns[column].left+l->rectangles[element].x);
				}
				s = maxColumnLength;
			}
		}
		minSize = max(s, orientation == HORIZONTAL ? e.minWidth : e.minHeight);
	}
	if ( orientation == HORIZONTAL ) {
		l->minimalWidths[element] = minSize;
	} else {
		l->minimalHeights[element] = minSize;
	}
	if (outMinSize) {
		*outMinSize = minSize;
	}
}

static void adjustChildrenRectangles_r(Layout* l, int index, int deltaX) {
	Rect & childRect = l->rectangles[index];
	childRect.x = childRect.x + deltaX;
	LayoutElement& e = l->elements[index];
	if (e.childrenList == -1) {
		return;
	}
	const vector<int>& children = l->childrenLists[e.childrenList].children;
	for(int i = 0; i < children.size(); ++i) {
		adjustChildrenRectangles_r(l, children[i], deltaX);
	}
}

void calculateRectangles_r(Layout* l, ElementType pass, int element, int length, int offset)
{
	LayoutElement& e = l->elements[element];
	Rect & out = l->rectangles[element];
	if ( pass == HORIZONTAL ) {
		out.x = offset;
		out.w = length;
	} else {
		out.y = offset;
		out.h = length;
	}

	if (e.type != HORIZONTAL &&
		e.type != VERTICAL &&
		e.type != MULTI_COLUMN) {
		return;
	}

	if (e.childrenList == -1) {
		return;
	}

	const vector<int>& children = l->childrenLists[e.childrenList].children;
	int count = (int)children.size();
	bool updateInPass = pass == HORIZONTAL ? e.type == HORIZONTAL : e.type == VERTICAL || e.type == MULTI_COLUMN;
	if ( !updateInPass) {
		// leave this level for other pass
		for (int i = 0; i < count; ++i) {
			LayoutElement& child = l->elements[children[i]];
			calculateRectangles_r(l, pass, children[i], length, offset);
		}
		return;
	}

	LayoutColumn columns[MAX_COLUMNS] = {
		{ 0, int(children.size()), 0, 0 }
	};
	int numColumns = 1;
	if (e.type == MULTI_COLUMN) {
		restoreColumns(&columns, &numColumns, *l, element);
	}

	int maxLength = 0;
	for (int columnIndex = 0; columnIndex < numColumns; ++columnIndex) {
		const LayoutColumn & column = columns[columnIndex];

		int availableLength = length;
		int expandingCount = 0;
		int countByPriority[MAX_PRIORITY] = { 0 };
		int lengthByPriority[MAX_PRIORITY+1] = { 0 };
		int totalFixedLength = 0;;
		for (int i = column.start; i < column.end; ++i) {
			LayoutElement& child = l->elements[children[i]];
			if (child.type != FIXED_SIZE &&
				child.type != HEIGHT_BY_WIDTH)
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
		for (int i = column.start; i < column.end; ++i) {
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
					int magnetLeft = max(0, l->magnetPoint - (offset + position + childLength));
					int freeDelta = min(magnetLeft, freeSpaceLeft);
					childLength += freeDelta;
					freeSpaceLeft -= freeDelta;
				}
				else if (pass == HORIZONTAL && child.type != FIXED_SIZE && child.type != HEIGHT_BY_WIDTH) {
					int freeDelta = expandingLeft ? freeSpaceLeft / expandingLeft : 0;
					childLength += freeDelta;
					freeSpaceLeft -= freeDelta;
				}
			}
			if (child.type != FIXED_SIZE && child.type != HEIGHT_BY_WIDTH) {
				--expandingLeft;
			}
			calculateRectangles_r(l, pass, children[i], childLength, offset+position);
			position += childLength;
		}
		maxLength = max(position, maxLength);
	}

	if ( pass == VERTICAL ) {
		out.h = maxLength;
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
	DebugTimer t( __FUNCTION__, 0 );
	Layout& l = *layout_;
	l.clear();

	// Width of our name/value column
	l.magnetPoint = int(area_.width() * (1.0f - valueColumnWidth()));

	// Allocate root layout element
	l.elements.push_back(LayoutElement());
	PropertyRow* root = model_->root();
	l.rows.push_back(root);
	int lroot = l.addElement(-1, VERTICAL, root, PART_CHILDREN_AREA, 0, 0, 0, false);
	root->setLayoutElement(lroot);
	// Root level and orphaned validators
	addValidatorsToLayout_r(this, &l, lroot, root);
	// Most of the layout is generated here
	populateChildrenArea(&l, lroot, root, this, 0);

	l.minimalWidths.resize(l.elements.size());
	l.minimalHeights.resize(l.elements.size());

	// Individual rectangles of the layout are computed in following steps:
	//
	// 1. Propagate horizontal minimal sizes of individual elements bottom up
	calculateMinimalSizes(0, HORIZONTAL, &l, lroot);
	// 2. Compute horizontal offsets and sizes top-down
	l.rectangles.resize(l.elements.size());
	calculateRectangles(&l, HORIZONTAL, lroot, area_.width(), 0);
	// 3. Propagate vertical minimal sizes of individual elements bottom up
	calculateMinimalSizes(0, VERTICAL, &l, lroot);
	// 4. Compute vertical offsets and sizes top-down
	calculateRectangles(&l, VERTICAL, lroot, 0, filterAreaHeight());
	//
	// Separating minimal size computation step allows to perform layout in O(n) instead
	// of O(n^2).
	//
	// Horizontal and vertical size computation is separated as some of the elements
	// height depends on their width.
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
	DebugTimer t("PropertyTree::revert", 10);
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
	DebugTimer t("PropertyTree::apply", 10);
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

static int findFirstFocusableElement(const property_tree::Layout& layout, int startingWithElement) {
	int numLayoutElements = (int)layout.elements.size();
	for ( int index = startingWithElement; index < numLayoutElements; ++index) {
		const property_tree::LayoutElement& element = layout.elements[index];
		if ( element.focusable ) {
			return index;
		}
	}
	return 0;
}

void PropertyTree::setFocusedRow(PropertyRow* row) {
	// make sure that the row is actually in layout now
	ensureVisible(row);

	int layoutElement = findFirstFocusableElement( *layout_, row->layoutElement() );
	if ( layoutElement > 0 ) {
		focusedLayoutElement_ = layoutElement;
		setSelectedRow(row);
	}
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
		updateHeights();
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
	updateHeights();
}

void PropertyTree::setPackCheckboxes(bool pack)
{
	style_->packCheckboxes = pack;
	updateHeights();
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
	DebugTimer t("PropertyTree::drawLayout", 20);
	int lastElement = 0;
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

