#include "StdAfx.h"
#include "ww/PopupMenu.h"
#include <algorithm>
#include "ww/Unicode.h"
#include "ww/Widget.h"
#include "ww/Win32/Window.h"
#include <windows.h>

namespace ww{

PopupMenu::PopupMenu(int maxItems)
{
    idRangeStart_ = ID_RANGE_MAX - maxItems;
    idRangeEnd_   = ID_RANGE_MAX;
}


PopupMenuItem* PopupMenu::nextItem(PopupMenuItem* item) const
{
    if(!item->children().empty())
        return item->children().front();
    else{
        PopupMenuItem0::Children& items = item->parent()->children();
        PopupMenuItem0::Children::iterator it = std::find(items.begin(), items.end(), item);
        ASSERT(it != items.end());
        while(item->parent() && *it == item->parent()->children().back()){
            item = item->parent();
			if(item->parent())
				it = std::find(item->parent()->children().begin(), item->parent()->children().end(), item);
        }
        if(item->parent()){
            ++it;
            item = *it;
            return item;
        }
        else
            return 0;
    }
}

void PopupMenu::assignIDs()
{
    DWORD currentID = idRangeStart_;
    PopupMenuItem* current = &root_;
    while(current){
        ASSERT(currentID < idRangeEnd_);
        if(current->children().empty())
            current->id_ = currentID++;
        else
            current->id_ = 0;

        current = nextItem(current);
    }
}

void PopupMenu::clear()
{
	root_.children().clear();
}

void PopupMenu::spawn(Widget* widget)
{
    POINT pt;
	GetCursorPos(&pt);
	spawn(Vect2(pt.x, pt.y), widget);
}

void PopupMenu::spawn(Vect2 point, Widget* widget)
{
	if(root_.children().empty())
		return;

    assignIDs();
	root_.setMenuHandle(::CreatePopupMenu());

    PopupMenuItem* current = &root_;
    while(current = nextItem(current)){
        if(current->children().empty()){
			UINT_PTR id = current->menuID();
			std::wstring text = toWideChar(current->text());
			if(text == L"-")
				AppendMenu(current->parent()->menuHandle(), MF_SEPARATOR, id, L"");
			else{
				if(current->hotkey() != KeyPress()){
					text += L"\t";
					text += toWideChar(current->hotkey().toString(true).c_str());
				}
				AppendMenu(current->parent()->menuHandle(), 
						   MF_STRING | (current->isChecked() ? MF_CHECKED : 0)
						   | (current->isEnabled() ? 0 : MF_GRAYED)
						   | (current->isDefault() ? MF_DEFAULT : 0),
						   id, text.c_str());			
			}
        }
		else{
			HMENU handle = ::CreatePopupMenu();
			current->setMenuHandle(handle);
			BOOL result = ::InsertMenu(current->parent()->menuHandle(), -1, MF_BYPOSITION | MF_POPUP, UINT_PTR(handle), toWideChar(current->text()).c_str());
			ASSERT(result == TRUE);
		}
    }

	current = &root_;
    while(current = nextItem(current)){
        if(!current->children().empty()){
			UINT_PTR handle = UINT_PTR(current->menuHandle());
		}
    }

	Win32::Window32* parentWindow = 0;
	if (widget)
		parentWindow = _findWindow(widget);
	HWND parentWindowHandle =  parentWindow ? parentWindow->get() : 0;
	ASSERT(parentWindowHandle == 0 || ::IsWindow(parentWindowHandle));

	UINT id = ::TrackPopupMenu(root_.menuHandle(), TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, 0, parentWindowHandle, 0);

	if(root_.children().empty())
		return;
	
    current = &root_;
    while(current = nextItem(current)){
        if(current->children().empty()){
			UINT current_id = current->menuID();
			if(current_id == id){
                current->call();
			}
        }
    }
}

// ---------------------------------------------------------------------------

PopupMenuItem::~PopupMenuItem()
{
	if(id_ && !children_.empty()){
		::DestroyMenu(menuHandle());
	}
}

PopupMenuItem0& PopupMenuItem::add(const char* text)
{
	PopupMenuItem0* item = new PopupMenuItem0(text);
	addChildren(item);
	return *item;
}


PopupMenuItem& PopupMenuItem::setHotkey(KeyPress key)
{
    hotkey_ = key;
	return *this;
}

PopupMenuItem& PopupMenuItem::addSeparator()
{
	return add("-");
}

PopupMenuItem* PopupMenuItem::find(const char* text)
{
    Children::iterator it;
    FOR_EACH(children_, it){
        PopupMenuItem* item = *it;
        if(strcmp(item->text(), text) == 0)
            return item;
    };
    return 0;
}

}
