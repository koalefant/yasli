#pragma once

#include "ww/Dialog.h"
#include "ww/Color.h"

namespace ww{

class ColorChooser;

class ColorChooserDialog : public ww::Dialog{
public:
	ColorChooserDialog(ww::Widget* parent, const Color& color = Color(0, 204, 0, 255), bool showColor = true, bool showAlpha = true);
	~ColorChooserDialog();

	void set(const Color& color);
	Color get() const{ return color_; }
protected:
	void onChooserChanged();

	ColorChooser* chooser_;
	Color color_;
};

}

