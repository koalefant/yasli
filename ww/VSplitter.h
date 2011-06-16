#pragma once

#include "ww/Splitter.h"

namespace ww{

class SplitterImpl;
class WW_API VSplitter : public Splitter{
public:
	VSplitter(int splitterSpacing = 1, int border = 0);
	~VSplitter();

protected:
	VSplitter(int splitterSpacing, int border, SplitterImpl* impl);
	bool vertical() { return true; }

	Widget* _nextWidget(Widget* last, FocusDirection focusDirection) const;
	int boxLength() const;
	Vect2i elementSize(Widget* widget) const;
	Rect rectByPosition(int start, int end);
	void setSplitterMinimalSize(const Vect2i& size);
	int positionFromPoint(const Vect2i point) const;
};

}

