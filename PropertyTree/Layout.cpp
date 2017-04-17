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
#include "PropertyTreeBase.h"
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

static bool isFullRow(const PropertyRow* row, const PropertyTreeBase* tree)
{
    if (tree->fullRowMode())
		return true;
	if (row->parent() && row->parent()->isContainer() && tree->fullRowContainers())
		return true;
	return row->userFullRow();
}

static void populateRowArea(bool* hasNonInlinedChildren, Layout* l, int rowArea, PropertyRow* row, PropertyTreeBase* tree, int indexForContainerElement, bool isInlined)
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
	for (int j = 0; j < count; ++j) {
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

	for (int j = 0; j < count; ++j) {
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


void addValidatorsToLayout_r(PropertyTreeBase* tree, Layout* l, int parentElement, PropertyRow* row)
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

void populateChildrenArea_r(Layout* l, int parentElement, PropertyRow* parentRow, PropertyTreeBase* tree, int indentationLevel)
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

static void calculateColumns(LayoutColumn (&columns)[MAX_COLUMNS], int* numColumns, int* maxColumnLength,
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
		LayoutColumn& column = columns[i];
		column.left = lastColumn;
		column.width = columnWidth;
		lastColumn = lastColumn + columnWidth;
	}
	columns[*numColumns-1].left = lastColumn;
	columns[*numColumns-1].width = widthAvailable - lastColumn;

	int startColumns[MAX_COLUMNS] = { 0 };
	int columnLength = 0;
	int totalLength = 0;
	int desiredLength = length / *numColumns;
	int currentColumn = 0;
	static vector<int> positions(count, 0); // FIXME: move to layout
	for (int i = 0; i < count; ++i) {
		int childIndex = children[i];
		LayoutElement& child = l->elements[childIndex];
		int childFixedLength = e.type == HORIZONTAL ? l->minimalWidths[childIndex] : l->minimalHeights[childIndex];
		positions[i] = totalLength;
		totalLength += childFixedLength;
		if (columnLength + childFixedLength > desiredLength && currentColumn + 1 < *numColumns) {
			startColumns[currentColumn] = i;
			++currentColumn;
			columnLength = childFixedLength;
		} else {
			columnLength += childFixedLength;
		}
	}

	// search for best column fit in small range
	// is there a better analytical solution?
	int offsets[] = { -1, 0, 1, -2, 2 };
	const int numOffsets = sizeof(offsets) / sizeof(offsets[0]);
	int bestColumnStarts[MAX_COLUMNS] = { 0 };
	int bestColumnLengths[MAX_COLUMNS] = { 0 };
	int leastDistanceSquareSum = INT_MAX;
	int maxOffsets = 1;
	int currentOffsetIndices[MAX_COLUMNS] = { 0 };
	printf("Possible columns:\n");
	if (*numColumns > 1) {
		while (true) {
			// increment current offset indices
			int column = 0;
			for (; column < *numColumns; ++column) {
				currentOffsetIndices[column] += 1;
				if (currentOffsetIndices[column] < numOffsets)
					break;
				currentOffsetIndices[column] = 0;
			}
			if (column == *numColumns) {
				break;
			}

			int currentColumns[MAX_COLUMNS-1];
			for (int j = 0; j < *numColumns - 1; ++j) {
				currentColumns[j] = startColumns[j] + offsets[currentOffsetIndices[j]];
			}
			
			// compute column lengths
			int columnLengths[MAX_COLUMNS];
			columnLengths[0] = positions[currentColumns[0]];
			for (int j = 1; j < *numColumns - 1; ++j) {
				columnLengths[j] = positions[currentColumns[j]] - positions[currentColumns[j-1]];
			}
			columnLengths[*numColumns - 1] = totalLength - positions[currentColumns[*numColumns - 2]];

			int error = 0;
			int idealColumn = totalLength / *numColumns;
			for (int j = 0; j < *numColumns; ++j) {
				int delta = columnLengths[j] - idealColumn;
				error += delta * delta;
			}

			if (error < leastDistanceSquareSum) {
				leastDistanceSquareSum = error;
				int lastLength = totalLength;
				for (int j = 0; j < *numColumns - 1; ++j) {
					bestColumnStarts[j] = currentColumns[j];
					lastLength -= columnLengths[j];
					bestColumnLengths[j] = columnLengths[j];
				}
				bestColumnLengths[*numColumns - 1] = lastLength;
			}
		}

		printf("best columns:\n");
		for (int i = 0; i < *numColumns; ++i) {
			printf("%d ", bestColumnLengths[i]);
			columns[i].start = i == 0 ? 0 : bestColumnStarts[i - 1];
			columns[i].end = i < *numColumns - 1 ? bestColumnStarts[i] : count;

			*maxColumnLength = max(*maxColumnLength, bestColumnLengths[i]);

			if (i > 0) {
				int childIndex = children[columns[i].start];
				LayoutElement& child = l->elements[childIndex];
				child.beginsColumn = true;
			}
		}
		printf("\n");

		for (int i = 0; i < *numColumns; ++i) {
			printf("column %d %d:%d w %d\n", i, columns[i].start, columns[i].end, columns[i].width);
		}
	} else {
		columns[0].start = 0;
		columns[0].end = count;
		*maxColumnLength = totalLength;
	}
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
				calculateColumns(*&columns, &numColumns, &maxColumnLength, &*l, element, rect.w);
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

int findRowElement(const Layout& l, const PropertyRow* row, int part, int subindex)
{
	int index = row->layoutElement();
	if (size_t(index) >= l.elements.size())
		return 0;
	for (; index < int(l.elements.size()); ++index) {
		const LayoutElement& element = l.elements[index];
		if (element.rowPart == part && element.rowPartSubindex == subindex && l.rows[index] == row)
			return index;
	}
	return 0;
}

int findFocusableRowElement(const Layout& l, const PropertyRow* row)
{
	int index = row->layoutElement();
	if (size_t(index) >= l.elements.size())
		return 0;
	for (; index < int(l.elements.size()); ++index) {
		const LayoutElement& element = l.elements[index];
		if (l.rows[index] == row && element.focusFlags != NOT_FOCUSABLE)
			return index;
	}
	return 0;
}

}
