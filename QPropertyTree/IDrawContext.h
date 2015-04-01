#pragma once

#include "Color.h"

namespace yasli { struct IconXPM; }

namespace property_tree {

enum Font
{
	FONT_NORMAL,
	FONT_BOLD
};

struct Rect;

struct IDrawContext
{
	virtual void drawIcon(const Rect& rect, const yasli::IconXPM& xpm) = 0;
	virtual void drawColor(const Rect& rect, const Color& color) = 0;
	virtual void drawComboBox(const Rect& rect, const char* text) = 0;
	virtual void drawSelection(const Rect& rect, bool inlinedRow) = 0;
	virtual void drawHorizontalLine(const Rect& rect) = 0;
	virtual void drawPlus(const Rect& rect, bool expanded, bool selected, bool grayed) = 0;
	virtual void drawLabel(const wchar_t* text, Font font, const Rect& rect, bool selected) = 0;
};

}

using namespace property_tree; // temporary
