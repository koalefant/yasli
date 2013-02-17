/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "Serialization.h"
#include "PropertyRowNumeric.h"

#define REGISTER_NUMERIC(TypeName, postfix) \
	typedef PropertyRowNumeric<TypeName> PropertyRow##postfix; \
	YASLI_CLASS(PropertyRow, PropertyRow##postfix, #TypeName);

using yasli::string;

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
PropertyRowWidgetNumeric::PropertyRowWidgetNumeric(PropertyRow* row, PropertyTreeModel* model, PropertyRowNumericInterface* numeric, QPropertyTree* tree)
: PropertyRowWidget(row, tree)
, numeric_(numeric)
, entry_(new QLineEdit())
, tree_(tree)
{
	entry_->setText(numeric_->valueAsString().c_str());
	connect(entry_, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

	entry_->selectAll();
}

void PropertyRowWidgetNumeric::onEditingFinished()
{
 tree_->model()->push(row());
 string str = entry_->text().toLocal8Bit().data();
	if(numeric_->setValueFromString(str.c_str()) || row_->multiValue())
		tree_->model()->rowChanged(row());
	else
		tree_->_cancelWidget();
}

void PropertyRowWidgetNumeric::commit()
{
	//if(entry_)
	//	entry_->commit();
}
