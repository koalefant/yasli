#include "StdAfx.h"
#include "yasli/Assert.h"
#include "ww/ColorRamp.h"
#include "ww/ColorChooser.h"
#include "ww/HBox.h"
#include "ww/VBox.h"
#include "ww/Slider.h"
#include "ww/ColorRect.h"
#include "ww/Label.h"
#include "ww/Entry.h"

namespace ww{

ColorChooser::ColorChooser(int border)
: VBox(5, border)
, showColor_(true)
, showAlpha_(true)
{
	relayout();
}

void ColorChooser::relayout()
{
	clear();

	sliderRed_ = 0;
	sliderGreen_ = 0;
	sliderBlue_ = 0;
	sliderAlpha_ = 0;
	entryRed_ = 0;
	entryGreen_ = 0;
	entryBlue_ = 0;
	entryAlpha_ = 0;
	entryHex_ = 0;
	ramp_ = 0;

	if(showColor_){
		ramp_ = new ColorRamp();
		ramp_->signalChanged().connect(this, &ColorChooser::onRampChanged);
		add(ramp_, PACK_FILL);
	}
	
	HBox* hbox = new HBox(5);
	add(hbox, PACK_COMPACT);
	{
        colorRect_ = new ColorRect();
		colorRect_->setRequestSize(Vect2i(64, 64));
		colorRect_->set(color_);
		hbox->add(colorRect_);

		VBox* vbox = new VBox();
		hbox->add(vbox, PACK_COMPACT);
		{
			if(showColor_){
				vbox->add(new Label("R"), PACK_FILL);
				vbox->add(new Label("G"), PACK_FILL);
				vbox->add(new Label("B"), PACK_FILL);
			}
			if(showAlpha_){
				vbox->add(new Label("A"), PACK_FILL);
			}
		}
		vbox = new VBox();
		hbox->add(vbox, PACK_FILL);
		{
			if(showColor_){
				sliderRed_ = new Slider();
				sliderRed_->setStepsCount(256);
				sliderRed_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderRed_, PACK_FILL);
				sliderGreen_ = new Slider();
				sliderGreen_->setStepsCount(256);
				sliderGreen_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderGreen_, PACK_FILL);
				sliderBlue_ = new Slider();
				sliderBlue_->setStepsCount(256);
				sliderBlue_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderBlue_, PACK_FILL);
			}
			if(showAlpha_){
				sliderAlpha_ = new Slider();
				sliderAlpha_->setStepsCount(256);
				sliderAlpha_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderAlpha_, PACK_FILL);
			}
		}
		vbox = new VBox();
		hbox->add(vbox, PACK_COMPACT);
		vbox->setRequestSize(Vect2i(50, 10));
		{
			if(showColor_){
				entryRed_ = new Entry();
				entryRed_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryRed_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryRed_, PACK_FILL);

				entryGreen_ = new Entry();
				entryGreen_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryGreen_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryGreen_, PACK_FILL);

				entryBlue_ = new Entry();
				entryBlue_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryBlue_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryBlue_, PACK_FILL);
			}
			if(showAlpha_){
				entryAlpha_ = new Entry();
				entryAlpha_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryAlpha_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryAlpha_, PACK_FILL);
			}
		}
	}
	if(showColor_ || showAlpha_){
		entryHex_ = new Entry();
		entryHex_->signalChanged().connect(this, &ColorChooser::onHexChanged);
		entryHex_->setTextAlign(ALIGN_CENTER);
		add(entryHex_);
	}
}

void ColorChooser::setShowColor(bool showColor)
{
	showColor_ = showColor;
	relayout();
}

void ColorChooser::setShowAlpha(bool showAlpha)
{
	showAlpha_ = showAlpha;
	relayout();
}

void ColorChooser::set(const Color4f& color)
{
	color_ = color;
	if(ramp_)
		ramp_->set(color);
	colorRect_->set(color);
	updateEntries(color);
	updateSliders(color);
	updateHex(color);
}

static void setEntryValue(Entry* entry, float value)
{
	char buf[16];
	sprintf_s(buf, sizeof(buf), "%i", round(value * 255)) ;
	entry->setText(buf);
}

void ColorChooser::updateEntries(const Color4f& color)
{
	if(showColor_){
		setEntryValue(entryRed_, color.r);
		setEntryValue(entryGreen_, color.g);
		setEntryValue(entryBlue_, color.b);
	}
	if(showAlpha_){
		setEntryValue(entryAlpha_, color.a);
	}
}

void ColorChooser::updateHex(const Color4f& color)
{
	if(showColor_ || showAlpha_){
		char buf[16];
		Color4c c = color;
		sprintf_s(buf, sizeof(buf), "%02X%02X%02X%02X", c.r, c.g, c.b, c.a);
		entryHex_->setText(buf);
	}
}

void ColorChooser::updateSliders(const Color4f& color)
{
	if(showColor_){
		sliderRed_->setValue(color.r);
		sliderGreen_->setValue(color.g);
		sliderBlue_->setValue(color.b);
	}
	if(showAlpha_){
		sliderAlpha_->setValue(color.a);
	}
}

void ColorChooser::onRampChanged()
{
	ASSERT(ramp_);
	color_ = Color4f(ramp_->get().r, ramp_->get().g, ramp_->get().b, color_.a);
	updateSliders(color_);
	updateEntries(color_);
	updateHex(color_);
	colorRect_->set(color_);
	signalChanged_.emit();
}

void ColorChooser::onSliderChanged()
{
	if(showColor_){
		color_.r = sliderRed_->value();
		color_.g = sliderGreen_->value();
		color_.b = sliderBlue_->value();
	}
	if(showAlpha_){
		color_.a = sliderAlpha_->value();
	}
	if(ramp_)
		ramp_->set(color_);
	colorRect_->set(color_);
	updateEntries(color_);
	updateHex(color_);
	signalChanged_.emit();
}

void ColorChooser::onEntryChanged()
{
	if(showColor_){
		color_.r = clamp((float)atof(entryRed_->text()) / 255.0f, 0.0f, 1.0f);
		color_.g = clamp((float)atof(entryGreen_->text()) / 255.0f, 0.0f, 1.0f);
		color_.b = clamp((float)atof(entryBlue_->text()) / 255.0f, 0.0f, 1.0f);
	}
	if(showAlpha_){
		color_.a = clamp((float)atof(entryAlpha_->text()) / 255.0f, 0.0f, 1.0f);
	}
	if(ramp_)
		ramp_->set(color_);
	colorRect_->set(color_);
	updateSliders(color_);
	updateHex(color_);
	signalChanged_.emit();
}

void ColorChooser::onHexChanged()
{
	int r = 255, g  = 255, b = 255, a = 255;
	sscanf_s(entryHex_->text(), "%2X%2X%2X%2X", &r, &g, &b, &a);
	color_ = Color4f(Color4c(r, g, b, a));
	colorRect_->set(color_);
	updateEntries(color_);
	updateSliders(color_);
	signalChanged_.emit();
}

}
