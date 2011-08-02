#pragma once
#include "ww/Widget.h"

namespace ww{

class WW_API Container : public Widget{
public:
	// virtuals:
	bool isVisible() const;
	void setBorder(int border);
	// ^^^

	// internal methods:
	virtual void _arrangeChildren() {}
protected:
};

}
