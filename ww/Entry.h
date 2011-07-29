#pragma once

#include <string>
#include "ww/_WidgetWithWindow.h"
#include "ww/_Enums.h"

namespace Win32{
	class Window;
}

namespace ww{
using std::string;
using std::wstring;
struct KeyPress;
class WW_API EntrySelection{
public:
	EntrySelection(int start = 0, int end = 0)
	: start_(start), end_(end)
	{
	}
	int start() const{ return start_; }
	int end() const{ return end_; }
	int length() const{ return end_ - start_; }
protected:
	int start_;
	int end_;
};


class EntryImpl;
class WW_API Entry : public _WidgetWithWindow{
public:
	explicit Entry(const wchar_t* text = L"", bool multline = false, int border = 0);
	explicit Entry(const char* text, bool multline = false, int border = 0);
	~Entry();

	TextAlignHorizontal textAlign() const{ return align_; }
	void setTextAlign(TextAlignHorizontal textAlign) { align_ = textAlign; }

	EntrySelection selection() const;
	void setSelection(EntrySelection selection);

	void replace(EntrySelection selection, const char* text);

	void setText(const char* text);
	void setText(const wchar_t* text);
	const char* text() const{ return text_.c_str(); }
	const wchar_t* textW() const{ return textW_.c_str(); }

	void setFlat(bool flat);
	bool flat() const{ return flat_; }

	void setMultiline(bool multiline);
	bool multiline() const{ return multiline_; }

	void setSwallowReturn(bool swallow){ swallowReturn_ = swallow; }

	void serialize(Archive& ar);

	virtual void onChanged() { signalChanged_.emit(); }
	signal0& signalChanged() { return signalChanged_; }

	void commit(); // calls onEdited, if changed
	virtual void onEdited() { signalEdited_.emit(); }
	signal0& signalEdited() { return signalEdited_; }

	virtual void onSelectionChanged() { signalSelectionChanged_.emit(); }
	sigslot::signal0& signalSelectionChanged() { return signalSelectionChanged_; }
protected:
    virtual bool onKeyPress(const KeyPress& key);

	EntryImpl* impl() const;
private:
	signal0 signalEdited_;
	signal0 signalChanged_;
	signal0 signalSelectionChanged_;

	string text_;
	wstring textW_;
	bool flat_;
    bool multiline_;
	TextAlignHorizontal align_;
	bool swallowReturn_;
	friend EntryImpl;
};

}

