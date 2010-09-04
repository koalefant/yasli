#pragma once

#include "ww/_WidgetWithWindow.h"
#include <string>

namespace Win32{
	class Window;
}


namespace ww{

class FrameImpl;

class WW_API Frame : public _ContainerWithWindow{
public:
	Frame(const char* text = "Frame", bool emphasis = true, int border = 0);
	~Frame();

	const char* text() const { return text_.c_str(); }
	void setText(const char* text);
	void setEmphasis(bool emphasis);
	bool emphasis() const{ return emphasis_; }

	void visitChildren(WidgetVisitor& visitor) const;
	/// добавить контрол в окно
	void add(Widget* widget);
	/// убирает единственный дочерний виджет
	void remove();

	void serialize(Archive& ar);

	void _arrangeChildren();
	void _relayoutParents();
	Widget* _nextWidget(Widget* last, FocusDirection focusDirection) const;
protected:

	SharedPtr<Widget> child_;
	std::string text_;
	int space_;
	bool emphasis_;
};

}

