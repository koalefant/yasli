#pragma once

#include "yasli/Assert.h"
#include <vector>
#include "PropertyTreeOperator.h"

class PropertyRow;

namespace property_tree
{
using std::vector;

// Number of collapse-prioritiies. 
// Used to hide individual cells in a row depending on their priority when
// there is not enough space to display all of them.
enum { MAX_PRIORITY = 4 };

// layout behavior of individual layout element
enum ElementType
{
	FIXED_SIZE,
	EXPANDING,
	HEIGHT_BY_WIDTH,
	EXPANDING_MAGNET,
	HORIZONTAL,
	VERTICAL,
};

// Role fulfilled by individual layout element
enum RowPart
{
	PART_INVALID,
	PART_INDENTATION_AND_CONTENT_AREA,
	PART_CHILDREN_AREA,
	PART_INDENTATION,
	PART_ROW_AREA,
	PART_LABEL,
	PART_WIDGET,
	PART_ARRAY_BUTTON,
	PART_EXTRA_VALUE_BUTTON,
	PART_PLUS,
	PART_VALIDATOR,
	PART_VALIDATOR_ERROR_ICON,
	PART_VALIDATOR_WARNING_ICON
};

// Describes one element in the layout
struct LayoutElement
{
	int childrenList;
	unsigned short minWidth;
	unsigned short minHeight;
	unsigned char type;
	unsigned char rowPart: 4;
	unsigned char rowPartSubindex: 4;
	unsigned char priority : 3;
	bool focusable : 1;

	LayoutElement()
	: type(FIXED_SIZE)
	, priority(0)
	, rowPart(PART_INVALID)
	, rowPartSubindex(0)
	, childrenList(-1)
	, minWidth(0)
	, minHeight(0)
	, focusable(false)
	{
	}
};

struct ChildrenList
{
	vector<int> children;
};


struct Layout
{
	PropertyTree* tree;
	// defines position where magnet elements get aligned to, used for name/value columns
	int magnetPoint;

	// Structure of Arrays below:
	// defines behavior of separate layout elements
	vector<LayoutElement> elements;
	// pointer to a row corresponding to an element
	vector<PropertyRow*> rows;
	// actual rectangle, result of the layout
	vector<Rect> rectangles;
	// minimal sizes considering children in the layout structure
	vector<int> minimalWidths;
	vector<int> minimalHeights;
	// ^^^ end of SoA
	
	// a cal back to calculate height of flowing elements (i.e. validator boxes)
	void * heightByWidthArgument;
	typedef int(*HeightByWidthFunction)(void *, int element, int subElement, int width);
	HeightByWidthFunction heightByWidth;

	// lists of children elements, referenced by individual LayoutElement
	vector<ChildrenList> childrenLists;
	int nextChildrenList;

	Layout()
	: tree()
    , nextChildrenList(0)
	, magnetPoint(0) {}

	// adds element to the layout
	int addElement(int parent, ElementType type, PropertyRow* row, RowPart part, int minWidth, int minHeight, int priority, bool focusable, int partSubindex = 0)
	{
		LayoutElement e;
		e.type = type;
		e.rowPart = part;
		e.rowPartSubindex = partSubindex;
		e.minWidth = minWidth;
		e.minHeight = minHeight;
		e.priority = priority;
		e.focusable = focusable;
		int index = (int)elements.size();
		elements.push_back(e);
		rows.push_back(row);

		if (parent != -1)
			addToChildrenList(parent, index);
		return index;
	}

	void addToChildrenList(int parent, int child)
	{
		if (size_t(parent) >= elements.size()) {
			YASLI_ASSERT(0);
			return;
		}
		LayoutElement& e = elements[parent];
		if (e.childrenList == -1) {
			e.childrenList = nextChildrenList;
			if (nextChildrenList >= childrenLists.size())
				childrenLists.push_back(ChildrenList());
			++nextChildrenList;
		}

		childrenLists[e.childrenList].children.push_back(child);
	}

	void clear()
	{
		elements.clear();
		rows.clear();
		for (size_t i = 0; i < childrenLists.size(); ++i)
			childrenLists[i].children.clear();
		nextChildrenList = 0;
		rectangles.clear();
		minimalWidths.clear();
		minimalHeights.clear();
	}
};

// result of a hit test through the layout
struct HitResult
{
	Point point;
	int elementIndex;
	int focusableElementIndex;
	RowPart part;
	int partIndex;
	Rect rect;
	PropertyRow* row;
};

struct PersistentLayoutElement
{
	TreePath row;
	RowPart part;
	int partIndex;

	PersistentLayoutElement()
	: part(PART_INVALID)
	, partIndex(-1)
	{
	}
};

};
