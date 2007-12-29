#include "StdAfx.h"
#include <wx/filedlg.h>
#include "yasli/Serializer.h"
#include "yasli/FileSelector.h"
#include "yasli/TypesFactory.h"
#include "PropertyItemFactory.h"
#include "PropertyTree.h"

#include "res/property_item_file_selector_button.xpm"

class PropertyItemFileSelector : public PropertyItemField{
public:
	PropertyItemFileSelector()
	: PropertyItemField("")
	{
	}

	PropertyItemFileSelector(const char* name, const Serializer& ser)
	: PropertyItemField(name, ser.type())
	, value_(*reinterpret_cast<FileSelector*>(ser.pointer()))
	{
		static wxBitmap bitmap((const char* const *)property_item_file_selector_button_xpm);
		addButton(&bitmap);
		ASSERT(ser.size() == sizeof(value_));
	}
	std::string toString() const{
		return value_.c_str();
	}
	bool activate(const ViewContext& context){
		ASSERT(value_.options());
        wxFileDialog dialog(context.tree, wxString("Choose a file", wxConvUTF8), wxEmptyString,
							wxString(value_.c_str(), wxConvUTF8),
							wxString(value_.options()->filter.c_str(), wxConvUTF8),
							wxFD_OPEN | wxFD_FILE_MUST_EXIST);

		if(dialog.ShowModal() == wxID_OK){
			value_ = dialog.GetFilename().utf8_str();
			context.tree->commitChange(this);
			return true;
		}
		return false;
	}
	void fromString(const char* str){
		value_ = str;
	}
	PROPERTY_ITEM_ASSIGN_IMPL(FileSelector)
protected:
	FileSelector value_;
};

REGISTER_PROPERTY_ITEM(FileSelector, PropertyItemFileSelector)
