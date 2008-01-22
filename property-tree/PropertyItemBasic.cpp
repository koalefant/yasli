#include "StdAfx.h"
#include <wx/combobox.h>
#include <wx/combo.h>
#include <wx/listctrl.h>
#include "yasli/TypesFactory.h"
#include "PropertyItemFactory.h"
#include "PropertyItemBasic.h"
#include "PropertyTree.h"

class PropertyControlCombo;

class wxListViewComboPopup : public wxListView,
                             public wxComboPopup
{
public:
    wxListViewComboPopup(PropertyControlCombo* control)
    : selfSelecting_(false)
    , control_(control)
    {
    }

    void onMouseMove(wxMouseEvent& event){
        wxPoint pt = event.GetPosition();
		selfSelecting_ = true;

        int flags = wxLIST_HITTEST_ONITEMRIGHT;
        int index = HitTest(pt, flags, 0);
        if(index != wxNOT_FOUND){
            if((flags & (wxLIST_HITTEST_ONITEM | wxLIST_HITTEST_ONITEMRIGHT)) != 0){
                wxListView::Select(index);
                index_ = index;
            }
        }
		selfSelecting_ = false;
    }
    void onMouseClick(wxMouseEvent& event);
	void onItemSelected(wxListEvent& event);
   
    // from wxComboPopup
    void Init(){
        index_ = -1;
    }
    bool Create(wxWindow* parent){
        return wxListView::Create(parent, 1, wxPoint(0, 0), wxDefaultSize,
                                  wxLC_LIST | wxLC_SINGLE_SEL | wxSIMPLE_BORDER);
    }
    wxWindow *GetControl(){ return this; }
    void OnDismiss();
    void OnComboDoubleClick();

    void SetStringValue(const wxString& s){
		selfSelecting_ = true;
        int n = wxListView::FindItem(-1, s);
        if(n >= 0 && n < wxListView::GetItemCount())
            wxListView::Select(n);
		selfSelecting_ = false;
    }

    wxString GetStringValue() const
    {
        if ( index_ >= 0 )
            return wxListView::GetItemText(index_);
        return wxEmptyString;
    }
protected:
    int index_;
	bool selfSelecting_;
    PropertyControlCombo* control_;
private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxListViewComboPopup, wxListView)
    EVT_MOTION(wxListViewComboPopup::onMouseMove)
    EVT_LEFT_UP(wxListViewComboPopup::onMouseClick)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, wxListViewComboPopup::onItemSelected)
END_EVENT_TABLE()

class PropertyControlCombo : public wxComboCtrl, public PropertyControl{
public:
    PropertyControlCombo(const PropertyItem::ViewContext& context,
                        PropertyItemStringListBase* item)
    : wxComboCtrl()
    , PropertyControl(item, context.tree)
    {

        wxListViewComboPopup* popupCtrl = new wxListViewComboPopup(this);
        SetPopupControl(popupCtrl);

        PropertyItemRects rects;
        item->calculateRects(context, rects);
        
		wxRect rect = scrollRect(item->calculateFieldRect(context, rects));
		rect.width += item->calculateButtonsRect(context, rects).width / item->buttonsCount();
		wxComboCtrl::Create(context.tree, wxID_ANY, wxT(""),
			                rect.GetPosition(), rect.GetSize(), wxCB_DROPDOWN | wxCB_READONLY);
		//SetSize();

        StringList::iterator it;
        StringList values;
        item->getStringList(values);
        FOR_EACH(values, it){
            const char* str = it->c_str();
            popupCtrl->InsertItem(popupCtrl->GetItemCount(), wxString(str, wxConvUTF8));
        }

        //popupCtrl->SetSelection(item->index());
		wxString text(item->toString().c_str(), wxConvUTF8);
        SetText(text);
		popupCtrl->SetStringValue(text);
        ShowPopup();
        //popupCtrl->SetItemState(item->index(), wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
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

void wxListViewComboPopup::onMouseClick(wxMouseEvent& WXUNUSED(event))
{
    index_ = wxListView::GetFirstSelected();

    //control_->commit();
    Dismiss();
}

void wxListViewComboPopup::onItemSelected(wxListEvent& event)
{
	if(selfSelecting_)
		return;
    index_ = wxListView::GetFirstSelected();

    //control_->commit();
    Dismiss();
}

void wxListViewComboPopup::OnDismiss()
{
    std::cout << "Dismissing..." << std::endl;
    control_->tree()->cancelControl(false); // add as pending message
}

void wxListViewComboPopup::OnComboDoubleClick()
{
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

REGISTER_PROPERTY_ITEM(StringListValue, PropertyItemStringList)

// ---------------------------------------------------------------------------
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemStruct, "struct")
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemString, "string")
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemInt, "int")
SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemFloat, "float")
