#include "QUIFacade.h"
#include <QFont>
#include <QFontMetrics>
#include "../QPropertyTree.h"
#include <QStyleOption>
#include <QDesktopWidget>
#include <QApplication>

namespace property_tree {


// ---------------------------------------------------------------------------

int QUIFacade::textWidth(const char* text, Font font)
{
	const QFont* qfont = font == FONT_BOLD ? &tree_->boldFont() : &tree_->font();

	QFontMetrics fm(*qfont);
	return fm.width(text);
}

Point QUIFacade::screenSize()
{
	QSize s = QApplication::desktop()->screenGeometry(tree_).size();
	return Point(s.width(), s.height());
}

IMenu* QUIFacade::createMenu()
{
	return new QtMenu("", tree_);
}

void QUIFacade::setCursor(Cursor cursor)
{
}

void QUIFacade::unsetCursor()
{
}

Point QUIFacade::cursorPosition()
{
	return tree_->mapFromGlobal(QCursor::pos());
}

}
