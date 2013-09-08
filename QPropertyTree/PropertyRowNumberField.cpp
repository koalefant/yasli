/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyRowNumberField.h"
#include "PropertyDrawContext.h"
#include <QStyleOption>
#include <QDesktopWidget>
#include <QApplication>

PropertyRowWidget* PropertyRowNumberField::createWidget(QPropertyTree* tree)
{
	return new PropertyRowWidgetNumber(tree->model(), this, tree);
}


void PropertyRowNumberField::redraw(const PropertyDrawContext& context)
{
    if(multiValue())
		context.drawEntry(L" ... ", false, true, 0);
    else 
	{
		QPainter* painter = context.painter;
		const QPropertyTree* tree = context.tree;

		QRect rt = context.widgetRect;
		rt.adjust(0, 0, 0, -1);

		QStyleOptionFrameV2 option;
		option.state = QStyle::State_Sunken;
		option.lineWidth = tree->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, 0);
		option.midLineWidth = 0;
		option.features = QStyleOptionFrameV2::None;

		if (context.captured) {
			option.state |= QStyle::State_HasFocus;
			option.state |= QStyle::State_Active;
			option.state |= QStyle::State_MouseOver;
		}
		else if (!userReadOnly()) {
			option.state |= QStyle::State_Enabled;
		}
		option.rect = rt; // option.rect is the rectangle to be drawn on.
		option.palette = tree->palette();
		option.fontMetrics = tree->fontMetrics();
		QRect textRect = tree->style()->subElementRect(QStyle::SE_LineEditContents, &option, 0);
		if (!textRect.isValid()) {
			textRect = rt;
			textRect.adjust(3, 1, -3, -2);
		}
		else {
			textRect.adjust(2, 1, -2, -1);
		}
		// some styles rely on default pens
		painter->setPen(QPen(tree->palette().color(QPalette::WindowText)));
		painter->setBrush(QBrush(tree->palette().color(QPalette::Base)));
		tree->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter, 0);
		painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString(valueAsString().c_str()), 0);
	}
}

bool PropertyRowNumberField::onMouseDown(QPropertyTree* tree, QPoint point, bool& changed)
{
	changed = false;
	if (widgetRect().contains(point) && !userReadOnly()) {
		startIncrement();
		return true;
	}
	return false;
}

void PropertyRowNumberField::onMouseDrag(const PropertyDragEvent& e)
{
	QCursor cur;
	cur.setShape(Qt::SizeHorCursor);
	e.tree->setCursor(cur);

	QSize screenSize = QApplication::desktop()->screenGeometry(e.tree).size();
	float relativeDelta = float((e.pos - e.start).x()) / screenSize.width();
	incrementLog(relativeDelta);
	setMultiValue(false);
}

void PropertyRowNumberField::onMouseStill(const PropertyDragEvent& e)
{
	e.tree->apply();
}

void PropertyRowNumberField::onMouseUp(QPropertyTree* tree, QPoint point) 
{
	tree->unsetCursor();
	endIncrement(tree);
}

bool PropertyRowNumberField::onActivate(QPropertyTree* tree, bool force)
{
	return false;
}

bool PropertyRowNumberField::onActivateRelease(QPropertyTree* tree)
{
	return tree->spawnWidget(this, false);
}

// ---------------------------------------------------------------------------

PropertyRowWidgetNumber::PropertyRowWidgetNumber(PropertyTreeModel* model, PropertyRowNumberField* row, QPropertyTree* tree)
: PropertyRowWidget(row, tree)
, row_(row)
, entry_(new QLineEdit())
, tree_(tree)
{
	//entry_->setAlignment(Qt::AlignCenter);
	entry_->setText(row_->valueAsString().c_str());
	connect(entry_, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

	entry_->selectAll();
}


void PropertyRowWidgetNumber::onEditingFinished()
{
	tree_->model()->rowAboutToBeChanged(row());
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

