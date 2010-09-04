#include "StdAfx.h"
#include "ww/ColorChooserDialog.h"
#include "ww/ColorChooser.h"
#include "yasli/Assert.h"

namespace ww{

ColorChooserDialog::ColorChooserDialog(ww::Widget* parent, const Color4f& color, bool showColor, bool showAlpha, int border)
: Dialog(parent, border)
{
	ASSERT(showColor || showAlpha);

	if(showColor)
		setTitle("Choose Color");
	else
		setTitle("Choose Alpha");

	if(showColor)
		setDefaultSize(Vect2i(350, 400));
	else
		setDefaultSize(Vect2i(400, 0));
	setResizeable(true);
	
	chooser_ = new ColorChooser();
	chooser_->signalChanged().connect(this, &ColorChooserDialog::onChooserChanged);
	chooser_->setShowColor(showColor);
	chooser_->setShowAlpha(showAlpha);
	add(chooser_, PACK_FILL);

	addButton("OK",     RESPONSE_OK);
	addButton("Cancel", RESPONSE_CANCEL);

	set(color);
}

void ColorChooserDialog::set(const Color4f& color)
{
	color_ = color;
	chooser_->set(color);
}

void ColorChooserDialog::onChooserChanged()
{
	color_ = chooser_->get();
}

}
