#pragma once

#include "yasli/Assert.h"
#include <vector>

class PropertyRow;

namespace property_tree
{
using std::vector;

enum
{
	MAX_PRIORITY = 4
};

enum ElementType
{
	FIXED_SIZE,
	EXPANDING,
	EXPANDING_MAGNET,
	HORIZONTAL,
	VERTICAL,
};

enum RowPart
{
	PART_INVALID,
	PART_INDENTATION_AND_CONTENT_AREA,
	PART_CONTENT_AREA,
	PART_INDENTATION,
	PART_ROW_AREA,
	PART_LABEL,
	PART_WIDGET,
	PART_ARRAY_BUTTON,
	PART_EXTRA_VALUE_BUTTON,
	PART_PLUS
};

struct LayoutElement
{
	unsigned char type;
	unsigned char rowPart: 4;
	unsigned char rowPartSubindex: 4;
	unsigned char priority : 4;
	int childrenList;
	unsigned short minWidth;
	unsigned short minHeight;

	LayoutElement()
	: type(FIXED_SIZE)
	, priority(0)
	, rowPart(PART_INVALID)
	, rowPartSubindex(0)
	, childrenList(-1)
	, minWidth(0)
	, minHeight(0)
	{
	}
};

struct LayoutOutput
{

};

struct ChildrenList
{
	vector<int> children;
};


struct Layout
{
	int magnetPoint;
	vector<LayoutElement> elements;
	vector<PropertyRow*> rows;
	vector<ChildrenList> childrenLists;
	vector<Rect> rectangles;
	vector<int> minimalWidths;
	vector<int> minimalHeights;
	int nextChildrenList;

	Layout()
	: nextChildrenList(0)
	, magnetPoint(0) {}

	int add(int parent, ElementType type, PropertyRow* row, RowPart part, int minWidth = 0, int minHeight = 0, int priority = 0)
	{
		LayoutElement e;
		e.type = type;
		e.rowPart = part;
		e.minWidth = minWidth;
		e.minHeight = minHeight;
		e.priority = priority;
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
};
