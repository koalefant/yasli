#pragma once

#include "ww/Box.h"

namespace ww{

class WW_API VBox : public Box{
public:
	VBox(int spacing = 0, int border = 0);
protected:
	Widget* _nextWidget(Widget* last, FocusDirection direction) const;
	void setElementPosition(Element& element, float offset, float length);
	float elementLength(const Element& element) const;
	float elementWidth(const Element& element) const;
	float boxLength() const;
	void setBoxSize(const Vect2& size);
};

}

