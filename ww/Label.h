#pragma once

#include <string>

#include "ww/_WidgetWithWindow.h"
#include "ww/_Enums.h"

namespace ww{

class LabelImpl;
class WW_API Label : public _WidgetWithWindow{
public:
	Label(const char* text = "Label", bool emphasis = false, int border = 0);

	/// виджет, которму передается фокус при нажатии хоткея из лейбла или клику по нему
	//void setMnemonicWidget(Widget* mnemonicWidget);

	void setAlignment(TextAlignHorizontal alignh, TextAlignVertical alignv = ALIGN_MIDDLE);

	void setExpandByContent(bool expand);
	bool expandByContent() const{ return expandByContent_; }

	void setEmphasis(bool emphasis);
	bool emphasis() const{ return emphasis_; }

	void setText(const char* text);
	const char* text() const{ return text_.c_str(); }

	void serialize(Archive& ar);
protected:
	LabelImpl* impl() const{ return reinterpret_cast<LabelImpl*>(_window()); }
	friend class LabelImpl;
	bool _focusable() const { return mnemonicWidget_ != 0; }

	std::string text_;
	Widget* mnemonicWidget_;
	TextAlignHorizontal alignHorizontal_;
	TextAlignVertical alignVertical_;
	bool emphasis_;
	bool expandByContent_;
};

};


