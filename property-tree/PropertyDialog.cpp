#include "StdAfx.h"
#include <wx/window.h>
#include <wx/sizer.h>
#include "PropertyDialog.h"
#include "PropertyTree.h"

IMPLEMENT_DYNAMIC_CLASS(wxPropertyDialog, wxDialog)
BEGIN_EVENT_TABLE(wxPropertyDialog, wxDialog)
END_EVENT_TABLE()

wxPropertyDialog::wxPropertyDialog(wxWindow* parent, const wxString& title)
: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(500, 600), 
		   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMAXIMIZE_BOX)
{
	Init();
}

wxPropertyDialog::wxPropertyDialog(wxWindow* parent, const wxString& title, Serializer& ser)
: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(500, 600),
		   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMAXIMIZE_BOX)
{
	Init();
	Set(ser);
}

void wxPropertyDialog::Init()
{
	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(boxSizer);
	{
		tree_ = new PropertyTree(this);
		boxSizer->Add(tree_, 1, wxEXPAND | wxALL, 3);

		wxSizer* sizer = CreateButtonSizer(wxOK | wxCANCEL);
		boxSizer->Add(sizer, 0, wxEXPAND | wxALL, 6);
	}
}

wxPropertyDialog::~wxPropertyDialog()
{
}

void wxPropertyDialog::Set(Serializer& serializer)
{
	tree_->attach(serializer);
	tree_->expandAll(1);
}
