/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyRowNumberField.h"
#include "PropertyDrawContext.h"
#include "QPropertyTree.h"
#include "IUIFacade.h"

InplaceWidget* PropertyRowNumberField::createWidget(PropertyTree* tree)
{
	return new InplaceWidgetNumber(tree->model(), this, (QPropertyTree*)tree);
}


void PropertyRowNumberField::redraw(PropertyDrawContext& context)
{
    if(multiValue())
		context.drawEntry(L" ... ", false, userReadOnly(), 0);
    else 
		context.drawNumberEntry(valueAsString().c_str(), context.widgetRect, context.captured, userReadOnly());
}

bool PropertyRowNumberField::onMouseDown(PropertyTree* tree, Point point, bool& changed)
{
	changed = false;
	if (widgetRect(tree).contains(point) && !userReadOnly()) {
		startIncrement();
		return true;
	}
	return false;
}

void PropertyRowNumberField::onMouseDrag(const PropertyDragEvent& e)
{
	e.tree->ui()->setCursor(CURSOR_SLIDE);

	Point screenSize = e.tree->ui()->screenSize();
	float relativeDelta = float((e.pos - e.start).x()) / screenSize.x();
	incrementLog(relativeDelta);
	setMultiValue(false);
}

void PropertyRowNumberField::onMouseStill(const PropertyDragEvent& e)
{
	e.tree->apply(true);
}

void PropertyRowNumberField::onMouseUp(PropertyTree* tree, Point point) 
{
	tree->ui()->unsetCursor();
	endIncrement(tree);
}

bool PropertyRowNumberField::onActivate(PropertyTree* tree, bool force)
{
	return false;
}

bool PropertyRowNumberField::onActivateRelease(PropertyTree* tree)
{
	return tree->spawnWidget(this, false);
}

// ---------------------------------------------------------------------------

InplaceWidgetNumber::InplaceWidgetNumber(PropertyTreeModel* model, PropertyRowNumberField* row, QPropertyTree* tree)
: InplaceWidget(row, tree)
, row_(row)
, entry_(new QLineEdit(tree))
, tree_(tree)
{
	//entry_->setAlignment(Qt::AlignCenter);
	entry_->setText(row_->valueAsString().c_str());
	connect(entry_, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

	entry_->selectAll();
}


void InplaceWidgetNumber::onEditingFinished()
{
	tree_->model()->rowAboutToBeChanged(row());
	yasli::string str = entry_->text().toLocal8Bit().data();
	if(row_->setValueFromString(str.c_str()) || row_->multiValue())
		tree_->model()->rowChanged(row());
	else
		tree_->_cancelWidget();
}

void InplaceWidgetNumber::commit()
{
	if(entry_)
		onEditingFinished();
}

