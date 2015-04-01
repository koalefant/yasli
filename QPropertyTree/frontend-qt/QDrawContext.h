#pragma once
#include "../PropertyDrawContext.h"

class QPropertyTree;
class QPainter;

namespace property_tree {

class QDrawContext : public PropertyDrawContext
{
public:
	QDrawContext(QPropertyTree* tree, QPainter* painter)
	: tree_(tree)
	, painter_(painter)
	{

		this->tree = tree;
		this->painter = painter;
	}

	void drawColor(const Rect& rect, const Color& color) override;
	void drawIcon(const Rect& rect, const yasli::IconXPM& xpm) override;
	void drawComboBox(const Rect& rect, const char* text) override;
	void drawSelection(const Rect& rect, bool inlinedRow) override;
	void drawHorizontalLine(const Rect& rect) override;
	void drawPlus(const Rect& rect, bool expanded, bool selected, bool grayed);
	void drawLabel(const wchar_t* text, Font font, const Rect& rect, bool selected);

private:
	QPropertyTree* tree_;
	QPainter* painter_;
};

}
