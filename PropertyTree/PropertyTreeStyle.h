#pragma once

#include "Serialization.h"
#include "yasli/decorators/Range.h"

struct PropertyTreeStyle
{
	bool compact;
	bool packCheckboxes;
	bool fullRowMode;
	bool horizontalLines;
	bool doNotIndentSecondLevel;
	bool groupRectangle;
	bool alignLabelsToRight;
	bool selectionRectangle;
	float valueColumnWidth;
	float rowSpacing;
	float firstLevelIndent;
	float levelIndent;
	float groupShade;
	float sliderSaturation;

	PropertyTreeStyle()
	: compact(false)
	, packCheckboxes(false)
	, fullRowMode(false)
	, valueColumnWidth(.59f)
	, rowSpacing(1.0f)
	, horizontalLines(true)
	, levelIndent(0.75f)
	, firstLevelIndent(0.75f)
	, doNotIndentSecondLevel(false)
	, sliderSaturation(0.0f)
	, groupShade(0.15f)
	, groupRectangle(false)
	, alignLabelsToRight(false)
	, selectionRectangle(true)
	{
	}

	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar)
	{
		using yasli::Range;
		ar.doc("Here you can define appearance of QPropertyTree control.");

		ar(valueColumnWidth, "valueColumnWidth", "Value Column Width");		ar.doc("Defines a ratio of the value / name columns. Normalized.");
		ar(Range(rowSpacing, 0.5f, 2.0f), "rowSpacing", "Row Spacing"); ar.doc("Height of one row (line) proportional to text-height.");
		ar(alignLabelsToRight, "alignLabelsToRight", "Right Alignment");
		ar(Range(firstLevelIndent, 0.0f, 3.0f), "firstLevelIndent", "First Level Indent"); ar.doc("Indentation of a very first level in text-height units.");
		ar(Range(levelIndent, 0.0f, 3.0f), "levelIndent", "Level Indent"); ar.doc("Indentation of a every next level in text-height units.");

		ar(Range(sliderSaturation, 0.0f, 1.0f), "sliderSaturation", "Slider Saturation");	

		ar(selectionRectangle, "selectionRectangle", "Selection Rectangle"); ar.doc("Show selection rectangle instead of just highlighting text.");
		ar(compact, "compact", "Compact"); ar.doc("Compact mode removes expansion pluses from the level and reduces inner padding. Useful for narrowing the widget.");
		ar(packCheckboxes, "packCheckboxes", "Pack Checkboxes"); ar.doc("Arranges checkboxes in two columns, when possible.");

		ar(horizontalLines, "horizontalLines", "Horizontal Lines"); ar.doc("Show thin line that connects row name with its value.");

		ar(doNotIndentSecondLevel, "doNotIndentSecondLevel", "Do not indent second level");	
		ar(groupRectangle, "groupRectangle", "Group Rectangle");	

		ar(Range(groupShade, -1.0f, 1.0f), "groupShade", groupRectangle ? "Group Shade" : 0); ar.doc("Shade of the group.");
	}
};

