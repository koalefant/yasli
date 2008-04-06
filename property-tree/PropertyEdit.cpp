#include "StdAfx.h"
#include <wx/app.h>
#include "PropertyEdit.h"
#include "property-tree/PropertyDialog.h"
#include "yasli/Serializer.h"

  class wxPropertyEditApp : public wxApp{
	 bool OnInit(){
		 return true;
	 }
 };

IMPLEMENT_APP_NO_MAIN(wxPropertyEditApp)

void propertyEdit(Serializer& serializer, int options)
{
    wxInitialize();
	{
		wxPropertyDialog dialog;
		dialog.Set(serializer);
		dialog.ShowModal();
	}
    wxUninitialize();
}
