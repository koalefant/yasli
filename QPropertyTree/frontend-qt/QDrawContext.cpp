#include "QDrawContext.h"
#include "../IDrawContext.h"
#include "../Rect.h"

#include "../QPropertyTree.h"
#include "../Unicode.h"
#include "../Color.h"
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QIcon>

void fillRoundRectangle(QPainter& p, const QBrush& brush, const QRect& _r, const QColor& border, int radius);

namespace property_tree {

static QColor interpolate(const QColor& a, const QColor& b, float k)
{
	float mk = 1.0f - k;
	return QColor(a.red() * k  + b.red() * mk,
				  a.green() * k + b.green() * mk,
				  a.blue() * k + b.blue() * mk,
				  a.alpha() * k + b.alpha() * mk);
}

static QColor toQColor(const Color& color)
{
	return QColor(color.r, color.g, color.b, color.a);
}


void QDrawContext::drawColor(const Rect& rect, const Color& color) override
{
	painter_->save();
	painter_->setBrush(QBrush(toQColor(color)));
	painter_->setPen(QPen(tree_->palette().color(QPalette::Shadow)));
	Rect rt = rect.adjusted(1,1,-1,-3);
	painter_->drawRoundedRect(rt, 2, 2);
	painter_->restore();
}

void QDrawContext::drawIcon(const Rect& rect, const yasli::IconXPM& xpm) override
{
	 QIcon icon(QPixmap::fromImage(*tree->_iconCache()->getImageForIcon(xpm)));
	 icon.paint(painter_, rect);
}

void QDrawContext::drawComboBox(const Rect& rect, const char* text)
{
	QStyleOptionComboBox option;
	option.editable = false;
	option.frame = true;
	option.currentText = QString(text);
	option.state |= QStyle::State_Enabled;
	option.rect = QRect(0, 0, rect.width(), rect.height());
	// we have to translate painter here to work around bug in some themes
	painter_->translate(rect.left(), rect.top());
	tree_->style()->drawComplexControl(QStyle::CC_ComboBox, &option, painter_);
	painter_->setPen(QPen(tree_->palette().color(QPalette::WindowText)));
	QRect textRect = tree_->style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, 0);
	textRect.adjust(1, 0, -1, 0);
	tree_->_drawRowValue(*painter_, toWideChar(text).c_str(), &tree_->font(), textRect, tree_->palette().color(QPalette::WindowText), false, false);
	painter_->translate(-rect.left(), -rect.top());
}

void QDrawContext::drawSelection(const Rect& rect, bool inlinedRow)
{
	if (inlinedRow) {
		QColor color1(tree->palette().button().color());
		QColor color2(tree->palette().highlight().color());
		QColor brushColor = tree->hasFocusOrInplaceHasFocus() ? interpolate(color1, color2, 0.33f) : tree->palette().shadow().color();
		fillRoundRectangle(*painter_, QBrush(brushColor), rect, brushColor, 6);
	}
	else {
		QBrush brush(tree_->hasFocusOrInplaceHasFocus() ? tree_->palette().highlight() : tree->palette().shadow());
		QColor brushColor = brush.color();
		QColor borderColor(brushColor.red(), brushColor.green(), brushColor.blue(), brushColor.alpha() / 4);
		fillRoundRectangle(*painter_, brush, rect, borderColor, 6);
	}
}

void QDrawContext::drawHorizontalLine(const Rect& rect)
{
	QLinearGradient gradient(rect.left(), rect.top(), rect.right(), rect.top());
	gradient.setColorAt(0.0f, tree->palette().color(QPalette::Button));
	gradient.setColorAt(0.6f, tree->palette().color(QPalette::Light));
	gradient.setColorAt(0.95f, tree->palette().color(QPalette::Light));
	gradient.setColorAt(1.0f, tree->palette().color(QPalette::Button));
	QBrush brush(gradient);
	painter_->fillRect(rect, brush);
}

void QDrawContext::drawPlus(const Rect& rect, bool expanded, bool selected, bool grayed)
{	
	QStyleOption option;
	option.rect = rect;
	option.state = QStyle::State_Enabled | QStyle::State_Children;
	if (expanded)
		option.state |= QStyle::State_Open;
	painter_->setPen(QPen());
	painter_->setBrush(QBrush());
	tree_->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter_, tree);
}

void QDrawContext::drawLabel(const wchar_t* text, Font font, const Rect& rect, bool selected)
{
	const QFont* qfont = font == FONT_NORMAL ? &tree_->font() : &tree_->boldFont();
	QColor textColor = tree->palette().buttonText().color();
	if(selected)
		textColor = tree->palette().highlightedText().color();
	tree_->drawFilteredString(*painter_, text, QPropertyTree::RowFilter::NAME, qfont, rect, textColor, false, false);
}

// ---------------------------------------------------------------------------

Rect::Rect(const QRect& rect)
: x(rect.left())
, y(rect.top())
, w(rect.width())
, h(rect.height())
{
}

Rect::operator QRect() const
{
	return QRect(x, y, w, h);
}

// ---------------------------------------------------------------------------

Point::Point(const QPoint& point)
: x_(point.x())
, y_(point.y())
{
}

Point::operator QPoint() const
{
	return QPoint(x_, y_);
}

}
