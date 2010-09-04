#pragma once

#include "ww/VBox.h"
#include "XMath/Colors.h"

namespace ww{

class ColorRamp;
class ColorRect;
class Entry;
class Slider;

class ColorChooser : public VBox{
public:
	ColorChooser(int border = 0);

	void set(const Color4f& color);
	const Color4f& get() const{ return color_; }

	void setShowColor(bool showColor);
	bool showColor()const{ return showColor_; }
	void setShowAlpha(bool showAlpha);
	bool showAlpha()const{ return showAlpha_; }

	sigslot::signal0& signalChanged() { return signalChanged_; }
protected:
	sigslot::signal0 signalChanged_;
	void relayout();
	void onRampChanged();
	void onSliderChanged();
	void onEntryChanged();
	void onHexChanged();
	void updateEntries(const Color4f& color);
	void updateSliders(const Color4f& color);
	void updateHex(const Color4f& color);
	
	bool showColor_;
	bool showAlpha_;

	Entry* entryRed_;
	Entry* entryGreen_;
	Entry* entryBlue_;
	Entry* entryAlpha_;
	Entry* entryHex_;

	Slider* sliderRed_;
	Slider* sliderGreen_;
	Slider* sliderBlue_;
	Slider* sliderAlpha_;

	ColorRamp* ramp_;
	ColorRect* colorRect_;
	Color4f color_;
};

}

