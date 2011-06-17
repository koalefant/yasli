#pragma once

namespace ww{

class Widget;

class WW_API Tooltip{
public:
	Tooltip(const char* text = "", bool baloon = false);
	void attach(Widget* widget);

	void setText(const char* text);
	const char* text() const { return text_.c_str(); }
	void show();
	void hide();

	void setBaloon(bool baloon);
	void setOffset(const Vect2& offset) { offset_ = offset; }

protected:
	std::string text_;
	bool baloon_;
	Widget* widget_;
	HWND toolTipWindow_;
	Vect2 offset_;
};

}
