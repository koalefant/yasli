/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowContainer.h"
#include "PropertyRowPointer.h"
#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "Serialization.h"
#include "PropertyRowPointer.h"

#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>

// ---------------------------------------------------------------------------

ContainerMenuHandler::ContainerMenuHandler(QPropertyTree* tree, PropertyRowContainer* container)
: container(container)
, tree(tree)
, pointerIndex(-1)
{
}

void ContainerMenuHandler::onMenuAppendElement()
{
	tree->model()->push(container);
	PropertyRow* defaultType = container->defaultRow(tree->model());
	YASLI_ESCAPE(defaultType != 0, return);
	PropertyRow* clonedRow = defaultType->clone();
	if(container->count() == 0)
		tree->expandRow(container);
	container->add(clonedRow);
	clonedRow->setLabelChanged();
	clonedRow->setLabelChangedToChildren();
	container->setMultiValue(false);
	if(container->expanded())
		tree->model()->selectRow(clonedRow, true);
	tree->expandRow(clonedRow);
	PropertyTreeModel::Selection sel = tree->model()->selection();
	tree->model()->rowChanged(clonedRow);
	tree->model()->setSelection(sel);
	tree->update(); 
	clonedRow = tree->selectedRow();
	if(clonedRow->activateOnAdd())
		clonedRow->onActivate(tree, false);
}

void ContainerMenuHandler::onMenuAppendPointerByIndex()
{
	PropertyRow* defaultType = container->defaultRow(tree->model());
	PropertyRow* clonedRow = defaultType->clone();
	// clonedRow->setFullRow(true); TODO
	if(container->count() == 0)
		tree->expandRow(container);
	container->add(clonedRow);
	clonedRow->setLabelChanged();
	clonedRow->setLabelChangedToChildren();
	container->setMultiValue(false);
	PropertyRowPointer* pointer = static_cast<PropertyRowPointer*>(clonedRow);
	if(container->expanded())
		tree->model()->selectRow(clonedRow, true);
	tree->expandRow(pointer);
	PropertyTreeModel::Selection sel = tree->model()->selection();

	CreatePointerMenuHandler handler;
	handler.tree = tree;
	handler.row = pointer;
	handler.index = pointerIndex;
	handler.onMenuCreateByIndex();
	tree->model()->setSelection(sel);
	tree->update(); 
}

void ContainerMenuHandler::onMenuRemoveAll()
{
	tree->model()->push(container);
	container->clear();
	tree->model()->rowChanged(container);
}

void ContainerMenuHandler::onMenuChildInsertBefore()
{
	tree->model()->push(container);
	PropertyRow* defaultType = tree->model()->defaultType(container->elementTypeName());
	if(!defaultType)
		return;
	PropertyRow* clonedRow = defaultType->clone();
	element->setSelected(false);
	container->addBefore(clonedRow, element);
	container->setMultiValue(false);
	tree->model()->selectRow(clonedRow, true);
	PropertyTreeModel::Selection sel = tree->model()->selection();
	tree->model()->rowChanged(clonedRow);
	tree->model()->setSelection(sel);
	tree->update(); 
	clonedRow = tree->selectedRow();
	if(clonedRow->activateOnAdd())
		clonedRow->onActivate(tree, false);
}

void ContainerMenuHandler::onMenuChildRemove()
{
	tree->model()->push(container);
	container->erase(element);
	container->setMultiValue(false);
	tree->model()->rowChanged(container);
}

// ---------------------------------------------------------------------------
YASLI_CLASS(PropertyRow, PropertyRowContainer, "Container");

PropertyRowContainer::PropertyRowContainer()
: fixedSize_(false)
, elementTypeName_("")
{
	buttonLabel_[0] = '\0';

}

PropertyRowContainer::PropertyRowContainer(const char* name, const char* label, const char* typeName)
: PropertyRow(name, label, typeName)
, fixedSize_(false)
, elementTypeName_("")
{
	buttonLabel_[0] = '\0';

	if(pulledUp())
		_setExpanded(true);		
}

struct ClassMenuItemAdderRowContainer : ClassMenuItemAdder
{
	ClassMenuItemAdderRowContainer(PropertyRowContainer* row, QPropertyTree* tree, bool insert = false) 
	: row_(row)
	, tree_(tree)
	, insert_(insert) {}    

	void addAction(QMenu& menu, const char* text, int index) override
	{
		ContainerMenuHandler* handler = new ContainerMenuHandler(tree_, row_);
		tree_->addMenuHandler(handler);
		handler->pointerIndex = index;

		QAction* action = menu.addAction(text);
		QObject::connect(action, SIGNAL(triggered()), handler, SLOT(onMenuAppendPointerByIndex()));
	}
protected:
	PropertyRowContainer* row_;
	QPropertyTree* tree_;
	bool insert_;
};

void PropertyRowContainer::redraw(const PropertyDrawContext& context)
{
	QRect widgetRect = context.widgetRect;
	if (widgetRect.width() == 0)
		return;
	QRect rt = widgetRect;
	rt.adjust(0, 1, -1, -1);
	QColor brushColor = context.tree->palette().button().color(); 
	QLinearGradient gradient(rt.left(), rt.top(), rt.left(), rt.bottom());
	gradient.setColorAt(0.0f, brushColor);
	gradient.setColorAt(0.6f, brushColor);
	gradient.setColorAt(1.0f, context.tree->palette().color(QPalette::Shadow));
	QBrush brush(gradient);

    const wchar_t* text = multiValue() ? L"..." : buttonLabel_;
	context.drawButton(rt, text, false, false);
}


bool PropertyRowContainer::onActivate( QPropertyTree* tree, bool force)
{
    if(userReadOnly())
        return false;
		QMenu menu;
    generateMenu(menu, tree);
    menu.exec(tree->_toScreen(QPoint(widgetPos_, pos_.y() + ROW_DEFAULT_HEIGHT)));
    return true;
}


void PropertyRowContainer::generateMenu(QMenu& menu, QPropertyTree* tree)
{
	ContainerMenuHandler* handler = new ContainerMenuHandler(tree, this);
	tree->addMenuHandler(handler);

	if (fixedSize_)
	{
		menu.addAction("[ Fixed Size Container ]")->setEnabled(false);
	}
  else if(userReadOnly())
  {
		menu.addAction("[ Read Only Container ]")->setEnabled(false);
	}
	else
	{
		PropertyRow* row = defaultRow(tree->model());
		if (row && row->isPointer())
		{
			QMenu* createItem = menu.addMenu("Add");
			menu.addSeparator();

			PropertyRowPointer* pointerRow = static_cast<PropertyRowPointer*>(row);
			ClassMenuItemAdderRowContainer(this, tree).generateMenu(*createItem, tree->model()->typeStringList(pointerRow->baseType()));
		}
		else
		{

			QAction* createItem = menu.addAction("Add");
			createItem->setShortcut(Qt::Key_Insert);
			QObject::connect(createItem, SIGNAL(triggered()), handler, SLOT(onMenuAppendElement()));

			menu.addSeparator();
		}

		QAction* removeAll = menu.addAction("Remove All");
		removeAll->setShortcut(Qt::Key_Delete);
		removeAll->setEnabled(!userReadOnly());
		QObject::connect(removeAll, SIGNAL(triggered()), handler, SLOT(onMenuRemoveAll()));
	}
}

bool PropertyRowContainer::onContextMenu(QMenu& menu, QPropertyTree* tree)
{
	if(!menu.isEmpty())
		menu.addSeparator();

	generateMenu(menu, tree);

	if(pulledUp())
		return !menu.isEmpty();

	return PropertyRow::onContextMenu(menu, tree);
}


PropertyRow* PropertyRowContainer::defaultRow(PropertyTreeModel* model)
{
	PropertyRow* defaultType = model->defaultType(elementTypeName_);
	//YASLI_ASSERT(defaultType);
	//YASLI_ASSERT(defaultType->numRef() == 1);
	return defaultType;
}

const PropertyRow* PropertyRowContainer::defaultRow(const PropertyTreeModel* model) const
{
	const PropertyRow* defaultType = model->defaultType(elementTypeName_);
	return defaultType;
}

void PropertyRowContainer::labelChanged()
{
    swprintf(buttonLabel_, sizeof(buttonLabel_)/sizeof(buttonLabel_[0]), L"%i", count());
}

void PropertyRowContainer::serializeValue(yasli::Archive& ar)
{
	ar(ConstStringWrapper(constStrings_, elementTypeName_), "elementTypeName", "ElementTypeName");
	ar(fixedSize_, "fixedSize", "fixedSize");
}

yasli::string PropertyRowContainer::valueAsString() const
{
	char buf[32] = { 0 };
    sprintf(buf, "%i", children_.size());
	return yasli::string(buf);
}


bool PropertyRowContainer::onKeyDown(QPropertyTree* tree, const QKeyEvent* ev)
{
	ContainerMenuHandler handler(tree, this);
	if(ev->key() == Qt::Key_Delete && ev->modifiers() == Qt::SHIFT){
		handler.onMenuRemoveAll();
		return true;
	}
	if(ev->key() == Qt::Key_Insert && ev->modifiers() == Qt::NoModifier){
		handler.onMenuAppendElement();
		return true;
	}
    return PropertyRow::onKeyDown(tree, ev);
}
