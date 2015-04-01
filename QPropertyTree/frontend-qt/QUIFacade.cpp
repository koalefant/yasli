#include "QUIFacade.h"
#include <QFont>
#include <QFontMetrics>
#include "../QPropertyTree.h"

namespace property_tree {

int QUIFacade::textWidth(const char* text, Font font)
{
	const QFont* qfont = font == FONT_BOLD ? &tree_->boldFont() : &tree_->font();

	QFontMetrics fm(*qfont);
	return fm.width(text);
}

}
