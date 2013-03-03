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
#include "PropertyRowNumberField.h"
#include "PropertyDrawContext.h"
#include <QtGui/QStyleOption>

PropertyRowNumberField::PropertyRowNumberField()
{
}

PropertyRowNumberField::PropertyRowNumberField(const char* name, const char* nameAlt, const char* typeName)
: PropertyRow(name, nameAlt, typeName)
{
}

PropertyRowNumberField::PropertyRowNumberField(const char* name, const char* nameAlt, const yasli::Serializer& ser)
: PropertyRow(name, nameAlt, ser)
{
}

PropertyRowWidget* PropertyRowNumberField::createWidget(QPropertyTree* tree)
{
	return new PropertyRowWidgetNumber(tree->model(), this, tree);
}


void PropertyRowNumberField::redraw(const PropertyDrawContext& context)
{
    if(multiValue())
		context.drawEntry(L" ... ", false, true);
    else 
	{
		QPainter* painter = context.painter;
		const QPropertyTree* tree = context.tree;

		QRect rt = context.widgetRect;
		rt.adjust(0, 0, 0, -1);

		QStyleOption option;
		option.state = QStyle::State_Sunken | QStyle::State_Editing;
		if (!userReadOnly())
			option.state |= QStyle::State_Enabled;
		option.rect = rt; // option.rect is the rectangle to be drawn on.
		QRect textRect = tree->style()->subElementRect(QStyle::SE_LineEditContents, &option, 0);
		if (!textRect.isValid())
		{
			textRect = rt;
			textRect.adjust(3, 1, -3, -2);
		}
		painter->fillRect(rt, tree->palette().base());
		tree->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter, tree);
		tree->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, painter, tree);
		painter->setPen(QPen(tree->palette().color(QPalette::WindowText)));
		painter->drawText(textRect, Qt::AlignCenter | Qt::AlignVCenter, QString(valueAsString().c_str()), 0);
	}
}

// ---------------------------------------------------------------------------

PropertyRowWidgetNumber::PropertyRowWidgetNumber(PropertyTreeModel* model, PropertyRowNumberField* row, QPropertyTree* tree)
: PropertyRowWidget(row, tree)
, row_(row)
, entry_(new QLineEdit())
, tree_(tree)
{
	entry_->setAlignment(Qt::AlignCenter);
	entry_->setText(row_->valueAsString().c_str());
	connect(entry_, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

	entry_->selectAll();
}


void PropertyRowWidgetNumber::onEditingFinished()
{
	tree_->model()->push(row());
	yasli::string str = entry_->text().toLocal8Bit().data();
	if(row_->setValueFromString(str.c_str()) || row_->multiValue())
		tree_->model()->rowChanged(row());
	else
		tree_->_cancelWidget();
}

void PropertyRowWidgetNumber::commit()
{
	//if(entry_)
	//	entry_->commit();
}

