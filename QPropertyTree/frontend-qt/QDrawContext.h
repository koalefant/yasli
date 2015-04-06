#pragma once
#include "../PropertyDrawContext.h"
#include "../QPropertyTree.h"

class QPropertyTree;
class QPainter;

namespace property_tree {

class QDrawContext : public IDrawContext
{
public:
	QDrawContext(QPropertyTree* tree, QPainter* painter)
	: tree_(tree)
	, painter_(painter)
	, iconCache_(new IconXPMCache())
	{

		this->tree = tree;
	}

	void drawButton(const Rect& rect, const wchar_t* text, bool pressed, bool focused, bool enabled, bool center, bool dropDownArrow, property_tree::Font font) override;
	void drawCheck(const Rect& rect, bool disabled, CheckState checked) override;
	void drawColor(const Rect& rect, const Color& color) override;
	void drawComboBox(const Rect& rect, const char* text) override;
	void drawEntry(const wchar_t* text, bool pathEllipsis, bool grayBackground, int trailingOffset) override;
	void drawHorizontalLine(const Rect& rect) override;
	void drawIcon(const Rect& rect, const yasli::IconXPM& icon) override;
	void drawLabel(const wchar_t* text, Font font, const Rect& rect, bool selected) override;
	void drawNumberEntry(const char* text, const Rect& rect, bool selected, bool grayed) override;
	void drawPlus(const Rect& rect, bool expanded, bool selected, bool grayed) override;
	void drawSelection(const Rect& rect, bool inlinedRow) override;
	void drawValueText(bool highlighted, const wchar_t* text) override;

private:
	QPropertyTree* tree_;
	QPainter* painter_;
	std::auto_ptr<IconXPMCache> iconCache_;
};

void fillRoundRectangle(QPainter& p, const QBrush& brush, const QRect& r, const QColor& borderColor, int radius);
void drawRoundRectangle(QPainter& p, const QRect &_r, unsigned int color, int radius, int width);

}
