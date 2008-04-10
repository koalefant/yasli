#include "StdAfx.h"
#include "yasli/Archive.h"
#include <wx/combobox.h>
#include <wx/combo.h>
#include <wx/listbox.h>
#include "yasli/TypesFactory.h"
#include "PropertyItemFactory.h"
#include "PropertyItemBasic.h"
#include "PropertyTree.h"

class PropertyControlCombo;

class wxListBoxComboPopup : public wxListBox,
                            public wxComboPopup
{
public:
    wxListBoxComboPopup(PropertyControlCombo* control)
    : selfSelecting_(false)
    , control_(control)
    {
    }

    void onMouseMove(wxMouseEvent& event){
        wxPoint pt = event.GetPosition();
		selfSelecting_ = true;

        int index = HitTest(pt);
        if(index != wxNOT_FOUND){
            wxListBox::Select(index);
            index_ = index;
        }
		selfSelecting_ = false;
    }
    void onMouseClick(wxMouseEvent& event);
	void onItemSelected(wxCommandEvent& event);	
   
    // from wxComboPopup
    void Init(){
        index_ = -1;
    }
    bool Create(wxWindow* parent){
        return wxListBox::Create(parent, 1, wxPoint(0, 0), wxDefaultSize, 0, 0,
                                 wxLB_NEEDED_SB | wxLB_SINGLE | wxSIMPLE_BORDER);
    }
    wxWindow *GetControl(){ return this; }
    void OnDismiss();
    void OnComboDoubleClick();

    void SetStringValue(const wxString& s){
		selfSelecting_ = true;
        int n = wxListBox::FindString(s);
        if(n >= 0 && n < int(wxListBox::GetCount()))
            wxListBox::Select(n);
		selfSelecting_ = false;
    }

    wxString GetStringValue() const
    {
        if ( index_ >= 0 )
            return GetString(index_);
        return wxEmptyString;
    }
protected:
    int index_;
	bool selfSelecting_;
    PropertyControlCombo* control_;
private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxListBoxComboPopup, wxListBox)
    EVT_MOTION(wxListBoxComboPopup::onMouseMove)
    EVT_LEFT_UP(wxListBoxComboPopup::onMouseClick)
    EVT_LISTBOX(wxID_ANY, wxListBoxComboPopup::onItemSelected)
END_EVENT_TABLE()

class PropertyControlCombo : public wxComboCtrl, public PropertyControl{
public:
    PropertyControlCombo(const PropertyItem::ViewContext& context, PropertyItemStringListBase* item)
    : wxComboCtrl()
    , PropertyControl(item, context.tree)
    {
        wxListBoxComboPopup* popupCtrl = new wxListBoxComboPopup(this);
        SetPopupControl(popupCtrl);

        PropertyItemRects rects;
        item->calculateRects(context, rects);
        
		wxRect rect = scrollRect(item->calculateFieldRect(context, rects));
		rect.width += item->calculateButtonsRect(context, rects).width / item->buttonsCount();
		wxComboCtrl::Create(context.tree, wxID_ANY, wxT(""),
			                rect.GetPosition(), rect.GetSize(), wxCB_DROPDOWN | wxCB_READONLY);

        StringList::iterator it;
        StringList values;
        item->getStringList(values);
        FOR_EACH(values, it){
            const char* str = it->c_str();
            popupCtrl->Append(wxString(str, wxConvUTF8));
        }
		SetPopupMaxHeight(popupCtrl->GetCount() * popupCtrl->GetCharHeight() + 2);

		wxString text(wxStringFromUTF8(item->toString().c_str()));
        SetText(text);
		popupCtrl->SetStringValue(text);
        ShowPopup();
        popupCtrl->SetFocus();
    }

    ~PropertyControlCombo(){
    }

    void commit(){
        PropertyItemStringListBase* item = safe_cast<PropertyItemStringListBase*>(property());
        ASSERT(item);
        item->fromString(GetValue().utf8_str());
		tree()->commitChange(item);
    }

    void onKillFocus(wxFocusEvent& event){
        //tree()->cancelControl();
    }

    wxWindow* get(){ return this; }

    DECLARE_CLASS(PropertyControlCombo)
    DECLARE_EVENT_TABLE()
protected:
};


IMPLEMENT_CLASS(PropertyControlCombo, wxComboCtrl)
BEGIN_EVENT_TABLE(PropertyControlCombo, wxComboCtrl)
    EVT_KILL_FOCUS(PropertyControlCombo::onKillFocus)
END_EVENT_TABLE()

void wxListBoxComboPopup::onMouseClick(wxMouseEvent& WXUNUSED(event))
{
    index_ = GetSelection();

    Dismiss();
}

void wxListBoxComboPopup::onItemSelected(wxCommandEvent& event)
{
	if(selfSelecting_)
		return;
    index_ = GetSelection();

    Dismiss();
}

void wxListBoxComboPopup::OnDismiss()
{
    control_->tree()->cancelControl(false); // add as pending message
}

void wxListBoxComboPopup::OnComboDoubleClick()
{
}

// ---------------------------------------------------------------------------

void PropertyItemBool::serializeValue(Archive& ar)
{
    ar(value_, "");
}

// ---------------------------------------------------------------------------

void PropertyItemInt::serializeValue(Archive& ar)
{
    ar(value_, "");
}

// ---------------------------------------------------------------------------

void PropertyItemLong::serializeValue(Archive& ar)
{
    ar(value_, "");
}

// ---------------------------------------------------------------------------

void PropertyItemFloat::serializeValue(Archive& ar)
{
    ar(value_, "");
}

// ---------------------------------------------------------------------------

void PropertyItemString::serializeValue(Archive& ar)
{
    ar(value_, "");
}

// ---------------------------------------------------------------------------

#include "res/property_item_combo_button.xpm"

PropertyItemStringListBase::PropertyItemStringListBase(const char* name)
: PropertyItemField(name, TypeID())
{
}

PropertyItemStringListBase::PropertyItemStringListBase(const char* name, TypeID type)
: PropertyItemField(name, type)
{
	static wxBitmap bitmap((const char**)property_item_combo_button_xpm);
	addButton(&bitmap);
}

PropertyControl* PropertyItemStringListBase::createControl(const ViewContext& context)
{
    return new PropertyControlCombo(context, this);
}
// ---------------------------------------------------------------------------


std::string PropertyItemStringListStatic::toString() const
{
    return value_.c_str();
}

void PropertyItemStringListStatic::fromString(const char* str)
{
    value_ = str;
}

void PropertyItemStringListStatic::serializeValue(Archive& ar)
{
    ar(Serializer(value_), "");
}

REGISTER_PROPERTY_ITEM(StringListStaticValue, PropertyItemStringListStatic)

// ---------------------------------------------------------------------------

std::string PropertyItemStringList::toString() const
{
    return value_.c_str();
}

void PropertyItemStringList::fromString(const char* str)
{
    value_ = str;
}

void PropertyItemStringList::serializeValue(Archive& ar)
{
    ar(Serializer(value_), "");
}

REGISTER_PROPERTY_ITEM(StringListValue, PropertyItemStringList)

// ---------------------------------------------------------------------------
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemStruct, "struct")
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemString, "string")
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemInt, "int")
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemFloat, "float")
