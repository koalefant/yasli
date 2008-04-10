#include "StdAfx.h"
#include "PopupMenu.h"
#include <iostream>

PopupMenuItem::PopupMenuItem(const char* text)
: text_(text)
, sensitive_(true)
{
}

PopupMenuItem& PopupMenuItem::add(PopupMenuItem* item)
{
    children_.push_back(item);
    return *item;
}

void PopupMenuItem::addSeparator()
{
    children_.push_back(new PopupMenuItem0("-"));
}

// ---------------------------------------------------------------------------

IMPLEMENT_CLASS(PopupMenu, wxMenu)
BEGIN_EVENT_TABLE(PopupMenu, wxMenu)
    EVT_MENU(ID_FIRST_COMMAND, PopupMenu::onMenuEvent)
    EVT_MENU(wxID_ANY, PopupMenu::onMenuEvent)
    EVT_MENU_RANGE(ID_FIRST_COMMAND, ID_LAST_COMMAND, PopupMenu::onMenuEvent)
END_EVENT_TABLE()

void PopupMenu::spawn(wxWindow* window, int x, int y)
{
	while(GetMenuItemCount() > 0)
        Remove(FindItemByPosition(0));
    wxMenu* menu = this;
    generateMenu(menu, 0, root_);
    if(x < 0)
        x = wxDefaultCoord;
    if(y < 0)
        y = wxDefaultCoord;
    window->PopupMenu(menu, x, y);
}

void PopupMenu::generateMenu(wxMenu* menu, wxMenu* subMenu, PopupMenuItem& item)
{
    handlers_.clear();
    PopupMenuItem::Children& children = item.children();
    PopupMenuItem::Children::iterator it;
    for(it = children.begin(); it != children.end(); ++it){
        PopupMenuItem* child = *it;
        ASSERT(child);
        if(child->empty()){
            int id = handlers_.size() + ID_FIRST_COMMAND;
            wxString text(child->text(), wxConvUTF8);
            wxMenuItem* menuItem = 0;
            if(!subMenu){
                if(text == wxT("-"))
                    menuItem = menu->AppendSeparator();
                else
                    menuItem = menu->Append(id, text);
            }
            else{
                if(text == wxT("-"))
                    menuItem = subMenu->AppendSeparator();
                else
                    menuItem = menu->Append(id, text, subMenu);
            }
            
            menuItem->Enable(child->sensitive());
            handlers_.push_back(child);
        }
        else{
            wxMenu* menuChild = new wxMenu();
            handlers_.push_back(0);
            generateMenu(menuChild, 0, *child);
            menu->AppendSubMenu(menuChild, wxString(child->text(), wxConvUTF8));
        }
    }
}

void PopupMenu::onMenuEvent(wxCommandEvent& event)
{
    int id = event.GetId();
    int index = id - ID_FIRST_COMMAND;
    ASSERT(index >= 0 && index < int(handlers_.size()));
    PopupMenuItem* item = handlers_[index];
    if(item)
        item->invoke();
}
