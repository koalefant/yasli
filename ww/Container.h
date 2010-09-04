#pragma once
#include "ww/Widget.h"

namespace ww{

class WW_API Container : public Widget{
public:
	// virtuals:
	bool isVisible() const;
	virtual bool isActive() const;
	void setBorder(int border);
	// ^^^

	// методы для внутреннего пользования:
	virtual void _arrangeChildren() {}
	void _setFocus();
	Widget* _nextWidget(Widget* last, FocusDirection direction) const;
	bool _focusable() const;
protected:
};

}