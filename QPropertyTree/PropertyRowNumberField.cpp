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
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>

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
		option.state = QStyle::State_Sunken/* | QStyle::State_Editing*/;
		if (context.captured) {
            // painter->fillRect(rt, tree->palette().highlight()); TODO: this doesn't work well on Ubuntu style
			option.state |= QStyle::State_HasFocus;
			option.state |= QStyle::State_Active;
			option.state |= QStyle::State_MouseOver;
		}
		else if (!userReadOnly()) {
            painter->fillRect(rt, tree->palette().base()); // TODO: remove this fill, doesn't work in other styles
			option.state |= QStyle::State_Enabled;
		}
		option.rect = rt; // option.rect is the rectangle to be drawn on.
		QRect textRect = tree->style()->subElementRect(QStyle::SE_FrameContents, &option, 0);
		if (!textRect.isValid()) {
			textRect = rt;
			textRect.adjust(3, 1, -3, -2);
		}
		tree->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter, 0);
		tree->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, painter, 0);
        painter->setPen(QPen(tree->palette().color(QPalette::WindowText)));
		painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString(valueAsString().c_str()), 0);
	}
}

bool PropertyRowNumberField::onMouseDown(QPropertyTree* tree, QPoint point, bool& changed)
{
	changed = false;
	if (widgetRect().contains(point)) {
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

