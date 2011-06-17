#include "StdAfx.h"
#include "ww/ColorChooserDialog.h"
#include "ww/ColorChooser.h"
#include "yasli/Assert.h"
#include "yasli/BinArchive.h"

namespace ww{

static std::string stateFileName = std::string(getenv("TEMP")) + "\\colorChooserDialog.tmp";

ColorChooserDialog::ColorChooserDialog(ww::Widget* parent, const Color4f& color, bool showColor, bool showAlpha, int border)
: Dialog(parent, border)
{
	ASSERT(showColor || showAlpha);

	if(showColor)
		setTitle("Choose Color");
	else
		setTitle("Choose Alpha");

	if(showColor)
		setDefaultSize(350, 400);
	else
		setDefaultSize(400, 0);
	setResizeable(true);

	chooser_ = new ColorChooser();
	chooser_->signalChanged().connect(this, &ColorChooserDialog::onChooserChanged);
	chooser_->setShowColor(showColor);
	chooser_->setShowAlpha(showAlpha);
	add(chooser_, PACK_FILL);

	addButton("OK",     RESPONSE_OK);
	addButton("Cancel", RESPONSE_CANCEL);

	set(color);

	BinIArchive ia;
	if(ia.load(stateFileName.c_str())){
		ia.setFilter(SERIALIZE_STATE);
		ia.serialize(*this, "window", 0);
	}
}

ColorChooserDialog::~ColorChooserDialog()
{
	BinOArchive oa;
	oa.setFilter(SERIALIZE_STATE);
	oa.serialize(*this, "window", 0);
	oa.save(stateFileName.c_str());
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
