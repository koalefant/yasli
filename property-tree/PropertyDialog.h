#pragma once

#include <wx/dialog.h>

class PropertyTree;
class Serializer;

class wxPropertyDialog : public wxDialog{
public:
	wxPropertyDialog(wxWindow* parent = 0, const wxString& title = wxEmptyString);
	wxPropertyDialog(wxWindow* parent, const wxString& title, Serializer& serializer);
	~wxPropertyDialog();

	void Set(Serializer& serializer);
protected:
	void Init();

    void onKeyDown(wxKeyEvent& event);

	PropertyTree* tree_;

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(wxPropertyDialog)
};

