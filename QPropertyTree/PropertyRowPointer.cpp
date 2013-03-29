/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowPointer.h"
#include "QPropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyDrawContext.h"
#include "Serialization.h"
#include "Unicode.h"
#include <QtGui/QMenu>


// ---------------------------------------------------------------------------

static QAction* findAction(const QList<QAction*>& actions, const char* withText)
{
	for (size_t i = 0; i < size_t(actions.size()); ++i)
	{
		if (actions[i]->text() == withText)
			return actions[i];
	}
	return 0;
}

void ClassMenuItemAdder::generateMenu(QMenu& createItem, const StringList& comboStrings)
{
	StringList::const_iterator it;
	int index = 0;
	for (it = comboStrings.begin(); it != comboStrings.end(); ++it) {
		StringList path;
		splitStringList(&path, it->c_str(), '\\');
		int level = 0;
		QMenu* item = &createItem;
		//createItem.addMenu(
		for(int level = 0; level < int(path.size()); ++level){
			const char* leaf = path[level].c_str();
			if(level == path.size() - 1){
				addAction(*item, leaf, index++);
			}
			else{
				QAction* action = 0;
				if(QAction* subItem = findAction(item->actions(), leaf))
					action = subItem;
				else
					item = addMenu(*item, leaf); //&item->add(leaf);
			}
		}
	}
}

void ClassMenuItemAdder::addAction(QMenu& menu, const char* text, int index)
{
	menu.addAction(text)->setEnabled(false);
}

QMenu* ClassMenuItemAdder::addMenu(QMenu& menu, const char* text)
{
	return menu.addMenu(text);
}


// ---------------------------------------------------------------------------

YASLI_CLASS(PropertyRow, PropertyRowPointer, "SharedPtr");

PropertyRowPointer::PropertyRowPointer()
: factory_(0)
{
}

void PropertyRowPointer::setDerivedType(const TypeID& typeID, yasli::ClassFactoryBase* factory)
{
	if (!factory) {
		derivedTypeName_.clear();
		return;
	}
	const yasli::TypeDescription* desc = factory->descriptionByType(typeID);
	if (!desc) {
		derivedTypeName_.clear();
		return;
	}
	derivedTypeName_ = desc->name();
}

TypeID PropertyRowPointer::getDerivedType(yasli::ClassFactoryBase* factory) const
{
	if (!factory)
		return TypeID();
	return factory->findTypeByName(derivedTypeName_.c_str());
}

bool PropertyRowPointer::assignTo(yasli::PointerInterface &ptr)
{
	TypeID derivedType = getDerivedType(ptr.factory());
	if ( ptr.type() != derivedType ) {
		ptr.create(derivedType);
	}

	return true;
}


void CreatePointerMenuHandler::onMenuCreateByIndex()
{
	tree->model()->push(row);
	if(index < 0){ // NULL value
		row->setDerivedType(TypeID(), 0);
		row->clear();
	}
	else{
		const PropertyDefaultTypeValue* defaultValue = tree->model()->defaultType(row->baseType(), index);
		if (defaultValue && defaultValue->root) {
			YASLI_ASSERT(defaultValue->root->refCount() == 1);
			if(useDefaultValue){
				row->clear();
				row->cloneChildren(row, defaultValue->root);
			}
			row->setDerivedType(defaultValue->type, row->factory());
			row->setLabelChanged();
			row->setLabelChangedToChildren();
			tree->expandRow(row);
		}
		else{
			row->setDerivedType(TypeID(), 0);
			row->clear();
		}
	}
	tree->model()->rowChanged(row);
}


yasli::string PropertyRowPointer::valueAsString() const
{
	yasli::string result;
	const yasli::TypeDescription* desc = 0;
	if (factory_)
		desc = factory_->descriptionByType(getDerivedType(factory_));
	if (desc)
		result = desc->label();
	else
		result = derivedTypeName_;

	return result;
}

yasli::wstring PropertyRowPointer::generateLabel() const
{
	if(multiValue())
		return L"...";

		yasli::wstring str;
		if(!derivedTypeName_.empty()){
			const char* textStart = derivedTypeName_.c_str();
			if (factory_) {
				const yasli::TypeDescription* desc = factory_->descriptionByType(getDerivedType(factory_));
				if (desc)
					textStart = desc->label();
			}
			const char* p = textStart + strlen(textStart);
			while(p > textStart){
				if(*(p - 1) == '\\')
					break;
				--p;
			}
			str = toWideChar(p);
			if(p != textStart){
				str += L" (";
				str += toWideChar(yasli::string(textStart, p - 1).c_str());
				str += L")";
			}
		}
	else
		{
				YASLI_ESCAPE(factory_ != 0, return L"NULL");
				str = toWideChar(factory_->nullLabel() ? factory_->nullLabel() : "[ null ]");
		}
		return str;
}

void PropertyRowPointer::redraw(const PropertyDrawContext& context)
{
	QRect widgetRect = context.widgetRect;
	QRect rt = widgetRect;
	rt.adjust(-1, 0, 0, 1);
	context.drawButton(rt, L"", false, false, !userReadOnly());

	const QPalette& pal = context.tree->palette();
	QColor textColor = userReadOnly() ? pal.shadow().color() : pal.windowText().color();
	QRect textRect = widgetRect;
	textRect.adjust(4, 0, -5, 0);
	yasli::wstring str = generateLabel();
	QFont* font = derivedTypeName_.empty() ? propertyTreeDefaultFont() : propertyTreeDefaultBoldFont();
	context.tree->_drawRowValue(*context.painter, str.c_str(), font, textRect, textColor, false, false);
}

struct ClassMenuItemAdderRowPointer : ClassMenuItemAdder{
	ClassMenuItemAdderRowPointer(PropertyRowPointer* row, QPropertyTree* tree) : row_(row), tree_(tree) {}    
	void addAction(QMenu& menu, const char* text, int index)
	{
		CreatePointerMenuHandler* handler = new CreatePointerMenuHandler;
		tree_->addMenuHandler(handler);
		handler->row = row_;
		handler->tree = tree_;
		handler->index = index;
		handler->useDefaultValue = !tree_->immediateUpdate();

		QAction* action = menu.addAction(text);

		QObject::connect(action, SIGNAL(triggered()), handler, SLOT(onMenuCreateByIndex()));
	}
protected:
	PropertyRowPointer* row_;
	QPropertyTree* tree_;
};


bool PropertyRowPointer::onActivate( QPropertyTree* tree, bool force)
{
		if(userReadOnly())
				return false;
		QMenu menu;
		ClassMenuItemAdderRowPointer(this, tree).generateMenu(menu, tree->model()->typeStringList(baseType()));
		menu.exec(tree->_toScreen(QPoint(widgetPos_, pos_.y() + ROW_DEFAULT_HEIGHT)));
		return true;
}

bool PropertyRowPointer::onMouseDown(QPropertyTree* tree, QPoint point, bool& changed) 
{
		if(widgetRect().contains(point)){
				if(onActivate(tree, false))
						changed = true;
		}
		return false; 
}

bool PropertyRowPointer::onContextMenu(QMenu &menu, QPropertyTree* tree)
{
	if(!menu.isEmpty())
		menu.addSeparator();
		if(!userReadOnly()){
			QMenu* createItem = menu.addMenu("Set");
			ClassMenuItemAdderRowPointer(this, tree).generateMenu(*createItem, tree->model()->typeStringList(baseType()));
		}
	return PropertyRow::onContextMenu(menu, tree);
}

void PropertyRowPointer::serializeValue(yasli::Archive& ar)
{
	ar(derivedTypeName_, "derivedTypeName", "Derived Type Name");
}

int PropertyRowPointer::widgetSizeMin() const
{
	QFontMetrics fm(*propertyTreeDefaultBoldFont());
    QString str(fromWideChar(generateLabel().c_str()).c_str());
	return fm.width(str) + 18;
}

void PropertyRowPointer::setValue(const yasli::PointerInterface& ptr)
{
	baseType_ = ptr.baseType();
	factory_ = ptr.factory();
	serializer_ = ptr.serializer();
	const yasli::TypeDescription* desc = factory_->descriptionByType(ptr.type());
	if (desc)
		derivedTypeName_ = desc->name();
}

// vim:ts=4 sw=4:
