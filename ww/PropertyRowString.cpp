/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <math.h>

#include "PropertyRowString.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTree.h"

#include "ww/Entry.h"

#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "ww/SafeCast.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "gdiplusUtils.h"

namespace ww{

// ---------------------------------------------------------------------------

class PropertyRowWidgetString : public PropertyRowWidget, public has_slots{
public:
    PropertyRowWidgetString(PropertyRowString* row, PropertyTree* tree)
	: PropertyRowWidget(row, tree->model())
    , initialValue_(row->value())
	, entry_(new Entry(row->value().c_str()))
    , tree_(tree)
	{
		entry_->signalEdited().connect(this, &PropertyRowWidgetString::onChange);
		entry_->setSelection(EntrySelection(0, -1));
		entry_->setSwallowReturn(true);
		entry_->setSwallowArrows(true);
		entry_->setSwallowEscape(true);
		entry_->setFlat(true);
	}
	~PropertyRowWidgetString()
	{
		entry_->signalEdited().disconnect_all();
	}

	void onChange(){
		PropertyRowString* row = safe_cast<PropertyRowString*>(this->row());
        if(initialValue_ != entry_->textW() || row_->multiValue()){
            model()->push(row);
		    row->setValue(entry_->textW());
		    model()->rowChanged(row);
        }
        else
            tree_->_cancelWidget();
	}
	void commit(){
		if(entry_)
			entry_->commit();
	}
	Widget* actualWidget() { return entry_; }
protected:
    PropertyTree* tree_;
	SharedPtr<Entry> entry_;
    wstring initialValue_;
};

// ---------------------------------------------------------------------------
YASLI_CLASS(PropertyRow, PropertyRowString, "string");

bool PropertyRowString::assignTo(string& str)
{
    str = fromWideChar(value_.c_str());
    return true;
}

bool PropertyRowString::assignTo(wstring& str)
{
    str = value_;
    return true;
}

PropertyRowWidget* PropertyRowString::createWidget(PropertyTree* tree)
{
	return new PropertyRowWidgetString(this, tree);
}

string PropertyRowString::valueAsString() const
{
	return fromWideChar(value_.c_str());
}

void PropertyRowString::setValue(const wchar_t* str)
{
	value_ = str;
}

void PropertyRowString::setValue(const char* str)
{
	value_ = toWideChar(str);
}

DECLARE_SEGMENT(PropertyRowString)
}

// vim:ts=4 sw=4:
