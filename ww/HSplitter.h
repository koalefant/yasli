#pragma once

#include "ww/Splitter.h"

namespace ww{
class SplitterImpl;
class WW_API HSplitter : public Splitter{
public:
	HSplitter(int splitterSpacing = 1, int border = 0);
	~HSplitter();
protected:
	HSplitter(int splitterSpacing, int border, SplitterImpl* impl);

	Widget* _nextWidget(Widget* last, FocusDirection focusDirection) const;
	bool vertical() { return false; }
	int boxLength() const;
	Vect2 elementSize(Widget* widget) const;
	Rect rectByPosition(int start, int end);
	void setSplitterMinimalSize(const Vect2& size);
	int positionFromPoint(const Vect2 point) const;
};

}

