#pragma once

#include "ww/Dialog.h"
#include "XMath/Colors.h"

namespace ww{

class ColorChooser;

class ColorChooserDialog : public ww::Dialog{
public:
	ColorChooserDialog(ww::Widget* parent, const Color4f& color = Color4f(0.0f, 0.8f, 0.0f, 1.0f), bool showColor = true, bool showAlpha = true, int border = 12);

	void set(const Color4f& color);
	const Color4f& get() const{ return color_; }
protected:
	void onChooserChanged();

	ColorChooser* chooser_;
	Color4f color_;
};

}

