#include "StdAfx.h"
#include "property-tree/wxSerialization.h"
#include "yasli/Archive.h"

#include <wx/window.h>
#include <wx/gdicmn.h>

struct wxRectSerializeable : wxRect{
	void serialize(Archive& ar){
		ar(x, "");
		ar(y, "");
		ar(width, "");
		ar(height, "");
	}
};	

bool serialize(Archive& ar, wxRect& rect, const char* name)
{
	return ar(static_cast<wxRectSerializeable&>(rect), name);
}

struct wxWindowSerializer{
	wxWindowSerializer(wxWindow& window)
	: window_(window)
	{}

	void serialize(Archive& ar)
	{
		wxRect rect = window_.GetRect();
		ar(rect, "rect");
		if(ar.isInput()){
			window_.Move(rect.GetPosition());
			window_.SetSize(rect.GetSize());
		}
	}

	wxWindow& window_;
};

bool serialize(Archive& ar, wxWindow& window, const char* name)
{
	wxWindowSerializer ser(window);
	return ar(ser, name);
}
