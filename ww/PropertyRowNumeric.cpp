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

#include "PropertyRowNumeric.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTree.h"

#include "ww/ConstStringList.h"
#include "ww/PopupMenu.h"
#include "ww/Entry.h"
#include "ClassMenu.h"

#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "yasli/Enum.h"
#include "ww/SafeCast.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "gdiplusUtils.h"

namespace ww{

// ---------------------------------------------------------------------------

#define REGISTER_NUMERIC(TypeName, postfix) \
	typedef PropertyRowNumeric<TypeName> PropertyRow##postfix; \
	YASLI_CLASS(PropertyRow, PropertyRow##postfix, #TypeName);

using ww::string;

REGISTER_NUMERIC(float, Float)
REGISTER_NUMERIC(double , Double)

REGISTER_NUMERIC(char, Char)
REGISTER_NUMERIC(signed char, SignedChar)
REGISTER_NUMERIC(unsigned char, UnsignedChar)

REGISTER_NUMERIC(short, Short)
REGISTER_NUMERIC(int, Int)
REGISTER_NUMERIC(long, Long)
REGISTER_NUMERIC(long long, LongLong)
REGISTER_NUMERIC(unsigned short, UnsignedShort)
REGISTER_NUMERIC(unsigned int, UnsignedInt)
REGISTER_NUMERIC(unsigned long, UnsignedLong)
REGISTER_NUMERIC(unsigned long long, UnsignedLongLong)

#undef REGISTER_NUMERIC

// ---------------------------------------------------------------------------
PropertyRowWidgetNumeric::PropertyRowWidgetNumeric(PropertyRow* row, PropertyTreeModel* model, PropertyRowNumericInterface* numeric, PropertyTree* tree)
: PropertyRowWidget(row, model)
, numeric_(numeric)
, entry_(new Entry(""))
, tree_(tree)
{
	entry_->setText(numeric_->valueAsString().c_str());

	entry_->signalEdited().connect(this, &PropertyRowWidgetNumeric::onChange);
	entry_->setSelection(EntrySelection(0, -1));
	entry_->setSwallowReturn(true);
	entry_->setSwallowArrows(true);
	entry_->setSwallowEscape(true);
	entry_->setFlat(true);
}

void PropertyRowWidgetNumeric::onChange()
{
    model()->push(row());
	if(numeric_->setValueFromString(entry_->text()) || row_->multiValue())
		model()->rowChanged(row());
	else
		tree_->_cancelWidget();
}

void PropertyRowWidgetNumeric::commit()
{
	if(entry_)
		entry_->commit();
}

}
// vim:ts=4 sw=4:
