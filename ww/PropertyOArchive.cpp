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
#include <memory>

#include "PropertyOArchive.h"
#include "ww/PropertyTreeModel.h"
#include "ww/PropertyDrawContext.h"
#include "ww/PropertyTree.h"

#include "ww/_PropertyRowBuiltin.h"
#include "ww/ConstStringList.h"
#include "ww/PopupMenu.h"
#include "ww/ComboBox.h"
#include "ww/CheckComboBox.h"
#include "ww/Entry.h"
#include "ClassMenu.h"

#include "yasli/BitVector.h"
#include "yasli/Archive.h"
#include "yasli/BitVectorImpl.h"
#include "yasli/ClassFactory.h"
#include "yasli/Enum.h"
#include "ww/SafeCast.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "ww/Color.h"
#include "gdiplusUtils.h"

#ifndef TRANSLATE
# define TRANSLATE(x) x
#endif

namespace ww{

void ClassMenuItemAdder::generateMenu(PopupMenuItem& createItem, const StringList& comboStrings)
{
	StringList::const_iterator it;
	int index = 0;
	FOR_EACH(comboStrings, it){
		StringList path;
		splitStringList(&path, TRANSLATE(it->c_str()), '\\');
		int level = 0;
		PopupMenuItem* item = &createItem;
		for(int level = 0; level < int(path.size()); ++level){
			const char* leaf = path[level].c_str();
			if(level == path.size() - 1){
				(*this)(*item, index++, leaf);
			}
			else{
				if(PopupMenuItem* subItem = item->find(leaf))
					item = subItem;
				else
					item = &item->add(leaf);
			}
		}
	}
}

// ---------------------------------------------------------------------------
YASLI_CLASS(PropertyRow, PropertyRowContainer, "Container");

PropertyRowContainer::PropertyRowContainer(const char* name = "", const char* label = "", const char* typeName = "", const char* elementTypeName = "", bool readOnly = false)
: PropertyRow(name, label, typeName)
, fixedSize_(readOnly)
, elementTypeName_(elementTypeName)
{
	buttonLabel_[0] = 0;

	if(pulledUp())
		_setExpanded(true);		
}

struct ClassMenuItemAdderRowContainer : ClassMenuItemAdder{
	ClassMenuItemAdderRowContainer(PropertyRowContainer* row, PropertyTree* tree, bool insert = false) 
	: row_(row)
	, tree_(tree)
	, insert_(insert) {}    

	void operator()(PopupMenuItem& root, int index, const char* text){
		if(!insert_)
			root.add(text, index, tree_).connect(row_, &PropertyRowContainer::onMenuAppendPointerByIndex);
	}
protected:
	PropertyRowContainer* row_;
	PropertyTree* tree_;
	bool insert_;
};

void PropertyRowContainer::redraw(const PropertyDrawContext& context)
{
	using namespace Gdiplus;
	using Gdiplus::Color;
	using Gdiplus::Rect;
	Rect widgetRect = gdiplusRect(context.widgetRect);
	if (widgetRect.Width == 0)
		return;
	Rect rt(widgetRect.X, widgetRect.Y + 1, widgetRect.Width - 2, widgetRect.Height - 2);
	Color brushColor = gdiplusSysColor(COLOR_BTNFACE);
	LinearGradientBrush brush(Rect(rt.X, rt.Y, rt.Width, rt.Height + 3), Color(255, 0, 0, 0), brushColor, LinearGradientModeVertical);

	Color colors[3] = { brushColor, brushColor, gdiplusSysColor(COLOR_3DSHADOW) };
	Gdiplus::REAL positions[3] = { 0.0f, 0.6f, 1.0f };
	brush.SetInterpolationColors(colors, positions, 3);

	fillRoundRectangle(context.graphics, &brush, rt, gdiplusSysColor(COLOR_3DSHADOW), 6);

	Color textColor;
    textColor.SetFromCOLORREF(userReadOnly() ? GetSysColor(COLOR_3DSHADOW) : GetSysColor(COLOR_WINDOWTEXT));
	Gdiplus::SolidBrush textBrush(textColor);
	RectF textRect( float(widgetRect.X), float(widgetRect.Y), float(widgetRect.Width), float(widgetRect.Height) );
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);
	format.SetFormatFlags(StringFormatFlagsNoWrap);
	format.SetTrimming(StringTrimmingNone);
	wchar_t* text = multiValue() ? L"..." : buttonLabel_; 
	context.graphics->DrawString(text, (int)wcslen(text), propertyTreeDefaultFont(), textRect, &format, &textBrush);
}

bool PropertyRowContainer::onKeyDown(PropertyTree* tree, KeyPress key)
{
	if(key == KeyPress(KEY_DELETE, KEY_MOD_SHIFT)){
		onMenuClear(tree->model());
		return true;
	}
	if(key == KeyPress(KEY_INSERT)){
		onMenuAppendElement(tree);
		return true;
	}
	return __super::onKeyDown(tree, key);
}

bool PropertyRowContainer::onActivate( PropertyTree* tree, bool force)
{
    if(userReadOnly())
        return false;
    PopupMenu menu;
    generateMenu(menu.root(), tree);
    menu.spawn(tree->_toScreen(Vect2(widgetPos_, pos_.y + ROW_DEFAULT_HEIGHT)), tree);
    return true;
}

void PropertyRowContainer::generateMenu(PopupMenuItem& root, PropertyTree* tree)
{
	if (fixedSize_)
	{
		root.add("[ Fixed Size Container ]").enable(false);
	}
    else if(userReadOnly())
    {
		root.add("[ Read Only Container ]").enable(false);
	}
	else
	{
		PopupMenuItem& createItem = root.add(TRANSLATE("Add"), tree)
			.connect(this, &PropertyRowContainer::onMenuAppendElement)
			.setHotkey(KeyPress(VK_INSERT));

		root.addSeparator();

		PropertyRow* row = defaultRow(tree->model());
		if(row && row->isPointer()){
			PropertyRowPointer* pointerRow = safe_cast<PropertyRowPointer*>(row);
			ClassMenuItemAdderRowContainer(this, tree).generateMenu(createItem, tree->model()->typeStringList(pointerRow->baseType()));
		}
		createItem.enable(true);

	    root.add(TRANSLATE("Remove All"), tree->model()).connect(this, &PropertyRowContainer::onMenuClear)
		    .setHotkey(KeyPress(KEY_DELETE, KEY_MOD_SHIFT))
		    .enable(!userReadOnly());
	}

}

bool PropertyRowContainer::onContextMenu(PopupMenuItem& root, PropertyTree* tree)
{
	if(!root.empty())
		root.addSeparator();

    generateMenu(root, tree);

	if(pulledUp())
		return !root.empty();

	return PropertyRow::onContextMenu(root, tree);
}


void PropertyRowContainer::onMenuClear(PropertyTreeModel* model)
{
    model->push(this);
	clear();
	model->rowChanged(this);
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


void PropertyRowContainer::onMenuAppendElement(PropertyTree* tree)
{
	tree->model()->push(this);
	PropertyRow* defaultType = defaultRow(tree->model());
	YASLI_ESCAPE(defaultType != 0, return);
	SharedPtr<PropertyRow> clonedRow = defaultType->clone();
	// clonedRow->setFullRow(true); TODO
	if(count() == 0)
		tree->expandRow(this);
	add(clonedRow);
	setMultiValue(false);
	if(expanded())
		tree->model()->selectRow(clonedRow, true);
	tree->expandRow(clonedRow);
	PropertyTreeModel::Selection sel = tree->model()->selection();
	tree->model()->rowChanged(this);
	tree->model()->setSelection(sel);
	tree->update(); 
	clonedRow = tree->selectedRow();
	if(clonedRow->activateOnAdd())
		clonedRow->onActivate(tree, false);
}

void PropertyRowContainer::onMenuAppendPointerByIndex(int index, PropertyTree* tree)
{
	PropertyRow* defaultType = defaultRow(tree->model());
	PropertyRow* clonedRow = defaultType->clone();
	// clonedRow->setFullRow(true); TODO
    if(count() == 0)
        tree->expandRow(this);
    add(clonedRow);
	setMultiValue(false);
	PropertyRowPointer* pointer = safe_cast<PropertyRowPointer*>(clonedRow);
	if(expanded())
		tree->model()->selectRow(clonedRow, true);
	tree->expandRow(pointer);
    PropertyTreeModel::Selection sel = tree->model()->selection();
	pointer->onMenuCreateByIndex(index, true, tree);
    tree->model()->setSelection(sel);
	tree->update(); 
}

void PropertyRowContainer::onMenuChildInsertBefore(PropertyRow* child, PropertyTree* tree)
{
    tree->model()->push(this);
	PropertyRow* defaultType = tree->model()->defaultType(elementTypeName_);
	if(!defaultType)
		return;
	PropertyRow* clonedRow = defaultType->clone();
	// clonedRow->setFullRow(true); TODO
	addBefore(clonedRow, child);
	setMultiValue(false);
	tree->model()->selectRow(clonedRow, true);
	PropertyTreeModel::Selection sel = tree->model()->selection();
	tree->model()->rowChanged(clonedRow);
	tree->model()->setSelection(sel);
	tree->update(); 
	clonedRow = tree->selectedRow();
	if(clonedRow->activateOnAdd())
		clonedRow->onActivate(tree, false);
}

void PropertyRowContainer::onMenuChildRemove(PropertyRow* child, PropertyTreeModel* model)
{
    model->push(this);
	erase(child);
	setMultiValue(false);
	model->rowChanged(this);
}

void PropertyRowContainer::digestReset(const PropertyTree* tree)
{
	swprintf_s(buttonLabel_, ARRAY_LEN(buttonLabel_), L"%i", count());

	/*
	wchar_t buffer[14];
	if(multiValue())
		swprintf_s(buffer, L"[...]");
	else
		swprintf_s(buffer, L"[ %i ]", int(count()));*/
	__super::digestReset(tree);
	//digestAppend(buffer);
}

void PropertyRowContainer::serializeValue(Archive& ar)
{
	ar(ConstStringWrapper(constStrings_, elementTypeName_), "elementTypeName", "ElementTypeName");
	ar(fixedSize_, "fixedSize", "fixedSize");
}

string PropertyRowContainer::valueAsString() const
{
    return numericAsString(children_.size());
}

// ---------------------------------------------------------------------------

class AutoDropComboBox : public ComboBox{
public:
	void show(){
		ComboBox::show();
		ComboBox::setFocus();
		showDropDown();
	}
};

// ---------------------------------------------------------------------------

class PropertyRowWidgetEnum : public PropertyRowWidget, public has_slots{
public:
	PropertyRowWidgetEnum(PropertyRowEnum* row, PropertyTreeModel* model)
	: PropertyRowWidget(row, model)
	, comboBox_(new AutoDropComboBox())
	{
		comboBox_->set(StringListValue( row->descriptor()->labels(), row->descriptor()->label(row->value()) ), true);
		comboBox_->signalEdited().connect(this, &PropertyRowWidgetEnum::onChange);
	}

	void onChange(){
		PropertyRowEnum* row = safe_cast<PropertyRowEnum*>(this->row());
        model()->push(row);
		row->setVisibleIndex(comboBox_->selectedIndex());
		model()->rowChanged(row);
	}
	void commit(){}
	Widget* actualWidget() { return comboBox_; }
protected:
	SharedPtr<ComboBox> comboBox_;
};

YASLI_CLASS(PropertyRow, PropertyRowEnum, "Enum");

PropertyRowEnum::PropertyRowEnum(const char* name, const char* label, int value, const EnumDescription* descriptor)
: PropertyRow(name, label, descriptor ? descriptor->type().name() : "")
, value_(value)
, descriptor_(descriptor)
{
}

void PropertyRowEnum::setVisibleIndex(int visibleIndex)
{
	const StringList& strings = descriptor()->labels();
	int index = 0;
	int vi = 0;
	while(index < int(strings.size())){
		if(strings[index].c_str()[0] != '\0'){
			if(vi == visibleIndex){
				value_ = descriptor()->valueByLabel(strings[index].c_str());
				return;
			}
			++vi;
		}
        ++index;
	}
	YASLI_ASSERT(0);
}

bool PropertyRowEnum::assignTo(void* object, size_t size)
{
	YASLI_ESCAPE(size == sizeof(int), return false);
	*reinterpret_cast<int*>(object) = value();
	return true;
}

int PropertyRowEnum::value() const
{
	return value_;
}

PropertyRowWidget* PropertyRowEnum::createWidget(PropertyTree* tree)
{
	return new PropertyRowWidgetEnum(this, tree->model());
}

void PropertyRowEnum::setValue(int value)
{
	value_ = value;
}

void PropertyRowEnum::serializeValue(Archive& ar)
{
	ar(value_, "value", "Значение");
}

string PropertyRowEnum::valueAsString() const{
	return descriptor_ ? descriptor_->label(value_) : "";
}

// ---------------------------------------------------------------------------

YASLI_CLASS(PropertyRow, PropertyRowPointer, "SharedPtr");

PropertyRowPointer::PropertyRowPointer()
: PropertyRow("", "", "")
, factory_(0)
{
}

PropertyRowPointer::PropertyRowPointer(const char* name, const char* label, const PointerInterface &ptr)
: PropertyRow(name, label, ptr.baseType().name())
, baseType_(ptr.baseType())
, derivedType_(ptr.type())
, factory_(ptr.factory())
{
    serializer_ = ptr.serializer();
}

PropertyRowPointer::PropertyRowPointer(const char* name, const char* label, TypeID baseType, TypeID derivedType, ClassFactoryBase* factory)
: PropertyRow(name, label, baseType.name())
, baseType_(baseType)
, derivedType_(derivedType)
, factory_(factory)
{
}

bool PropertyRowPointer::assignTo(PointerInterface &ptr)
{
	if ( ptr.type() != derivedType_ )
	{
		ptr.create(derivedType_);
	}
    return true;
}


void PropertyRowPointer::onMenuCreateByIndex(int index, bool useDefaultValue, PropertyTree* tree)
{
    tree->model()->push(this);
	if(index < 0){ // NULL value
		derivedType_ = TypeID();
	    clear();
	}
	else{
		YASLI_ASSERT(typeName_);
		const PropertyDefaultTypeValue* defaultValue = tree->model()->defaultType(baseType_, index);
		if (defaultValue && defaultValue->root) {
			YASLI_ASSERT(defaultValue->root->refCount() == 1);
            if(useDefaultValue){
                clear();
				cloneChildren(this, defaultValue->root);
            }
			derivedType_ = defaultValue->type;
			tree->expandRow(this);
		}
        else{
            derivedType_ = TypeID();
            clear();
        }
	}
	tree->model()->rowChanged(this);
}


string PropertyRowPointer::valueAsString() const
{
    string result = derivedType_.name();
		if (factory_) {
			const TypeDescription* desc = factory_->descriptionByType(derivedType_);
			if (desc)
				result = desc->label();
		}
    if(digest()[0] != L'\0'){
        result += " ( ";
        result += fromWideChar(digest());
        result += " )";
    }
	return result;
}

wstring PropertyRowPointer::generateLabel() const
{
	if(multiValue())
		return L"...";

    wstring str;
	if(derivedType_){
		const char* textStart = derivedType_.name();
		if (factory_) {
			const TypeDescription* desc = factory_->descriptionByType(derivedType_);
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
			str += toWideChar(string(textStart, p - 1).c_str());
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
	using namespace Gdiplus;
	using Gdiplus::Color;
	using Gdiplus::Rect;
	Rect widgetRect = gdiplusRect(context.widgetRect);

	Gdiplus::Rect rt(widgetRect.X, widgetRect.Y + 1, widgetRect.Width - 2, widgetRect.Height - 2);
	Color brushColor = gdiplusSysColor(COLOR_BTNFACE);
	LinearGradientBrush brush(Gdiplus::Rect(rt.X, rt.Y, rt.Width, rt.Height + 3), Color(255, 0, 0, 0), brushColor, LinearGradientModeVertical);

	Color colors[3] = { brushColor, brushColor, gdiplusSysColor(COLOR_3DSHADOW) };
	Gdiplus::REAL positions[3] = { 0.0f, 0.6f, 1.0f };
	brush.SetInterpolationColors(colors, positions, 3);

	fillRoundRectangle(context.graphics, &brush, rt, gdiplusSysColor(COLOR_3DSHADOW), 6);

	ww::Color textColor;
    textColor.setGDI(userReadOnly() ? GetSysColor(COLOR_3DSHADOW) : GetSysColor(COLOR_WINDOWTEXT));

	ww::Rect textRect(widgetRect.X + 4, widgetRect.Y, widgetRect.GetRight() - 5, widgetRect.GetBottom());
	wstring str = generateLabel();
	const wchar_t* text = str.c_str();;
	//context.drawValueText(false, text);
	Gdiplus::Font* font = derivedType_ == TypeID() ? propertyTreeDefaultFont() : propertyTreeDefaultBoldFont();
	context.tree->_drawRowValue(context.graphics, text, font, textRect, textColor, false, false);
}

struct ClassMenuItemAdderRowPointer : ClassMenuItemAdder{
	ClassMenuItemAdderRowPointer(PropertyRowPointer* row, PropertyTree* tree) : row_(row), tree_(tree) {}    
	void operator()(PopupMenuItem& root, int index, const char* text){
		root.add(text, index, !tree_->immediateUpdate(), tree_).connect(row_, &PropertyRowPointer::onMenuCreateByIndex);
	}
protected:
	PropertyRowPointer* row_;
	PropertyTree* tree_;
};


bool PropertyRowPointer::onActivate( PropertyTree* tree, bool force)
{
    if(userReadOnly())
        return false;
    PopupMenu menu;
    ClassMenuItemAdderRowPointer(this, tree).generateMenu(menu.root(), tree->model()->typeStringList(baseType()));
    menu.spawn(tree->_toScreen(Vect2(widgetPos_, pos_.y + ROW_DEFAULT_HEIGHT)), tree);
    return true;
}

bool PropertyRowPointer::onMouseDown(PropertyTree* tree, Vect2 point, bool& changed) 
{
    if(widgetRect().pointInside(point)){
        if(onActivate(tree, false))
            changed = true;
    }
    return false; 
}

bool PropertyRowPointer::onContextMenu(PopupMenuItem &menu, PropertyTree* tree)
{
	if(!menu.empty())
		menu.addSeparator();
    if(!userReadOnly()){
	    PopupMenuItem0& createItem = menu.add(TRANSLATE("Set"));
	    ClassMenuItemAdderRowPointer(this, tree).generateMenu(createItem, tree->model()->typeStringList(baseType()));
    }

	return PropertyRow::onContextMenu(menu, tree);
}

void PropertyRowPointer::serializeValue(Archive& ar)
{
	if (factory_) {
		TypeIDWithFactory pair(derivedType_, factory_);
		ar(pair, "derivedType", "DerivedType");
		if (ar.isInput())
			derivedType_ = pair.type;
	}
}

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

PropertyRowString::PropertyRowString(const char* name, const char* label, const wchar_t* value)
: PropertyRowImpl<wstring, PropertyRowString>(name, label, value, "string")
{
}

PropertyRowString::PropertyRowString(const char* name, const char* label, const char* value)
: PropertyRowImpl<wstring, PropertyRowString>(name, label, toWideChar(value), "string")
{
}

PropertyRowString::PropertyRowString(void* object, size_t size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<wstring, PropertyRowString>(object, size, name, label, typeName)
{

}

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

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
REGISTER_PROPERTY_ROW(StringListValue, PropertyRowStringListValue)

class PropertyRowWidgetStringListValue : public PropertyRowWidget, public has_slots{
public:
	PropertyRowWidgetStringListValue(PropertyRowStringListValue* row, PropertyTreeModel* model)
	: PropertyRowWidget(row, model)
	, comboBox_(new AutoDropComboBox())
	{
		comboBox_->set(row->value(), false);
		comboBox_->signalEdited().connect(this, &PropertyRowWidgetStringListValue::onChange);
	}
	PropertyRowWidgetStringListValue(PropertyRowStringListStaticValue* row, PropertyTreeModel* model)
	: PropertyRowWidget(row, model)
	, comboBox_(new AutoDropComboBox())
	{
		comboBox_->set(StringListValue(row->value()), false);
		comboBox_->signalEdited().connect(this, &PropertyRowWidgetStringListValue::onChange);
	}

	~PropertyRowWidgetStringListValue()
	{
	}

	void onChange(){
		if ( PropertyRowStringListValue* row = dynamic_cast<PropertyRowStringListValue*>(this->row()) )
		{
            model()->push(row);
			StringListValue comboList;
			comboBox_->get(comboList);
			row->setValue(comboList);
			model()->rowChanged(row);
		}
		else if ( PropertyRowStringListStaticValue* row = dynamic_cast<PropertyRowStringListStaticValue*>(this->row()) )
		{
            model()->push(row);
			StringListStaticValue comboList = row->value();
			comboBox_->get(comboList);
			row->setValue(comboList);
			model()->rowChanged(row);
		}
	}
	void commit(){}
	Widget* actualWidget() { return comboBox_; }
protected:
	SharedPtr<ComboBox> comboBox_;
};

PropertyRowStringListValue::PropertyRowStringListValue(const char* name, const char* label, const StringListValue& value)
: PropertyRowImpl<StringListValue, PropertyRowStringListValue>(name, label, value)
{
}

PropertyRowStringListValue::PropertyRowStringListValue(void* object, size_t size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<StringListValue, PropertyRowStringListValue>(object, size, name, label, typeName)
{
}

PropertyRowWidget* PropertyRowStringListValue::createWidget(PropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree->model());
}

// ---------------------------------------------------------------------------
REGISTER_PROPERTY_ROW(StringListStaticValue, PropertyRowStringListStaticValue)

PropertyRowStringListStaticValue::PropertyRowStringListStaticValue(const char* name, const char* label, const StringListStaticValue& value)
: PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>(name, label, value)
{
}

PropertyRowStringListStaticValue::PropertyRowStringListStaticValue(void* object, size_t size, const char* name, const char* label, const char* typeName)
: PropertyRowImpl<StringListStaticValue, PropertyRowStringListStaticValue>(object, size, name, label, typeName)
{
}

PropertyRowWidget* PropertyRowStringListStaticValue::createWidget(PropertyTree* tree)
{
	return new PropertyRowWidgetStringListValue(this, tree->model());
}


// ---------------------------------------------------------------------------


void setUpdatedRecurse(PropertyRow* row, bool updated)
{
	row->setUpdated(updated);
	PropertyRow::iterator it;
	FOR_EACH(*row, it){
		PropertyRow* row = static_cast<PropertyRow*>(&**it);
		setUpdatedRecurse(row, updated);
	}
}

PropertyOArchive::PropertyOArchive(PropertyTreeModel* model, PropertyRow* root)
: Archive(OUTPUT | EDIT)
, model_(model)
, currentNode_(root)
, lastNode_(0)
, updateMode_(false)
, defaultValueCreationMode_(false)
, rootNode_(root)
{
	YASLI_ASSERT(model != 0);
	if(!rootNode()->empty()){
		updateMode_ = true;
		setUpdatedRecurse(rootNode(), false);
	}
}



PropertyOArchive::PropertyOArchive(PropertyTreeModel* model, bool forDefaultType)
: Archive(OUTPUT | EDIT)
, model_(model)
, currentNode_(0)
, lastNode_(0)
, updateMode_(false)
, defaultValueCreationMode_(forDefaultType)
, rootNode_(0)
{
}

PropertyOArchive::~PropertyOArchive()
{
}

PropertyRow* PropertyOArchive::rootNode()
{
	if(rootNode_)
		return rootNode_;
	else{
		YASLI_ASSERT(model_);
		YASLI_ASSERT(model_->root());
		return model_->root();
	}
}

void PropertyOArchive::enterNode(PropertyRow* row)
{
	currentNode_ = row;
}

void PropertyOArchive::closeStruct(const char* name)
{
	if(currentNode_){
		lastNode_ = currentNode_;
		currentNode_ = currentNode_->parent();
		if(lastNode_ && updateMode_){
			PropertyRow::iterator it;
			for(it = lastNode_->begin(); it != lastNode_->end();){
				PropertyRow* row = static_cast<PropertyRow*>(&**it);
				if(!row->updated())
					it = lastNode_->erase(it);
				else
					++it;
			}
		}
		lastNode_->setLabelChanged();
	}
}

PropertyRow* PropertyOArchive::addRow(SharedPtr<PropertyRow> newRow, bool block, PropertyRow* previousNode)
{
    YASLI_ESCAPE(newRow, return 0);
	const char* label = newRow->label();
	if(!previousNode) // FIXME перенести в место вызова
		previousNode = lastNode_;

	PropertyRow* result = 0;
	if(currentNode_ == 0){
		if(updateMode_){
			newRow->swapChildren(rootNode());
			newRow->setParent(0);
				model_->setRoot(newRow);
			newRow->setUpdated(true);
			newRow->setLabel(label);
			result = newRow;
		}
		else{
			if(defaultValueCreationMode_)
				rootNode_ = newRow;
			else
				model_->setRoot(newRow);
			result = newRow;
		}
		return result;
	}
	else{
		if(updateMode_ || block){
			PropertyRow* row = currentNode_->find(newRow->name(), 0, newRow->typeName(), !block);

			// we need this to preserve order
			if(row && previousNode && previousNode->parent() == currentNode_){
				if(currentNode_->childIndex(row) != currentNode_->childIndex(previousNode) + 1){
					//newRow = row;
					currentNode_->erase(row);
					row = 0;
				}
			}

			if(row){
				currentNode_->replaceAndPreserveState(row, newRow);
				newRow->setUpdated(true);
				YASLI_ASSERT(newRow->parent() == currentNode_);
				result = newRow;
				result->setLabel(label);
			}
			else{
				if(model_->expandLevels() != 0){
					if(model_->expandLevels() == -1 ||
						 model_->expandLevels() >= currentNode_->level())
						newRow->_setExpanded(true);
				}
				result = currentNode_->add(&*newRow, previousNode);
				result->setLabel(label);
				newRow->setUpdated(true);
			}
		}
		else{
            if(model_->expandLevels() != 0){
                if(model_->expandLevels() == -1 ||
                   model_->expandLevels() >= currentNode_->level())
                    newRow->_setExpanded(true);
            }
			result = currentNode_->add(&*newRow);
		}
		return result;
	}
}

bool PropertyOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
    const char* typeName = ser.type().name();
    size_t size = ser.size();

	PropertyRowFactory& factory = PropertyRowFactory::the();
	SharedPtr<PropertyRow> row = factory.create(typeName, PropertyRowArg(ser.pointer(), size, name, label, typeName));
 	if(!row)
		row = new PropertyRow(name, label, ser);
	
	if(!row->isLeaf() || currentNode_ == 0){
		SharedPtr<PropertyRow> previousRow = lastNode_;
		lastNode_ = currentNode_;
		enterNode(addRow(row, false, previousRow));

		if(currentNode_->isLeaf())
			return false;
	}
	else{
		lastNode_ = addRow(row);
		return true;
	}

	if (ser)
		ser(*this);

    closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(StringInterface& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowString(name, label, value.get()));
	return true;
}

bool PropertyOArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowString(name, label, value.get()));
	return true;
}

bool PropertyOArchive::operator()(bool& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowBool(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(char& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<char>(name, label, value));
	return true;
}

// ---

bool PropertyOArchive::operator()(signed char& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed char>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(signed short& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed short>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(signed int& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed int>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(signed long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<signed long>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(long long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<long long>(name, label, value));
	return true;
}

// ---

bool PropertyOArchive::operator()(unsigned char& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned char>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned short& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned short>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned int& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned int>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned long>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(unsigned long long& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<unsigned long long>(name, label, value));
	return true;
}

// ---

bool PropertyOArchive::operator()(float& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<float>(name, label, value));
	return true;
}

bool PropertyOArchive::operator()(double& value, const char* name, const char* label)
{
	lastNode_ = addRow(new PropertyRowNumeric<double>(name, label, value));
	return true;
}


bool PropertyOArchive::operator()(ContainerInterface& ser, const char *name, const char *label)
{
	const char* elementTypeName = ser.type().name();
	bool fixedSizeContainer = ser.isFixedSize();
	PropertyRow* container = new PropertyRowContainer(name, label, ser.type().name(), elementTypeName, fixedSizeContainer);
	lastNode_ = currentNode_;
	enterNode(addRow(container, false));

	if (!model_->defaultTypeRegistered(elementTypeName)) {
		PropertyOArchive ar(model_, true);
		ar.setFilter(getFilter());
		model_->addDefaultType(0, elementTypeName); // add empty default to prevent recursion
		ser.serializeNewElement(ar, "default", "[+]");
		if (ar.rootNode() != 0)
			model_->addDefaultType(ar.rootNode(), ser.type().name());
	}

	if ( ser.size() > 0 )
		while( true )
		{
			ser(*this, "", "<");
			if ( !ser.next() )
				break;
		}

	closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(PointerInterface& ptr, const char *name, const char *label)
{
	lastNode_ = currentNode_;
	PropertyRow* row = new PropertyRowPointer(name, label, ptr);
	enterNode(addRow(row));

	{
		TypeID baseType = ptr.baseType();
		yasli::ClassFactoryBase* factory = ptr.factory();
		size_t count = factory->size();		

		const char* nullLabel = factory->nullLabel();
		if (!(nullLabel && nullLabel[0] == '\0'))
		{
			PropertyDefaultTypeValue nullValue;
			nullValue.type = TypeID();
			nullValue.factory = factory;
			nullValue.factoryIndex = -1;
			nullValue.label = nullLabel ? nullLabel : "[ null ]";
			model_->addDefaultType(baseType, nullValue);
		}

		for(size_t i = 0; i < count; ++i) {
			const TypeDescription *desc = factory->descriptionByIndex((int)i);
			if (!model_->defaultTypeRegistered(baseType, desc->typeID())){
				PropertyOArchive ar(model_, true);
				//ar.setInnerContext(getInnerContext());
				ar.setContextMap(contextMap());
				ar.setFilter(getFilter());

				PropertyDefaultTypeValue defaultValue;
				defaultValue.type = desc->typeID();
				defaultValue.factory = factory;
				defaultValue.factoryIndex = int(i);
				defaultValue.label = desc->label();

				model_->addDefaultType(baseType, defaultValue);
				factory->serializeNewByIndex(ar, (int)i, "name", "label");
				if (ar.rootNode() != 0) {
					ar.rootNode()->setTypeName(desc->name());
					defaultValue.root = ar.rootNode();
					model_->addDefaultType(baseType, defaultValue);
				}
			}
		}
	}

	if(Serializer ser = ptr.serializer())
		ser(*this);
	currentNode_->setLabelChanged();
	closeStruct(name);
	return true;
}

bool PropertyOArchive::operator()(Object& obj, const char *name, const char *label)
{
	const char* typeName = obj.type().name();

	PropertyRowObject* row = 0;
	if (typeName_.empty())
		 row = new PropertyRowObject(name, label, obj, model_);
	else
		 row = new PropertyRowObject(name, label, Object(0, obj.type(), 0, 0, 0), model_);
	lastNode_ = addRow(row);
	return true;
}

bool PropertyOArchive::openBlock(const char* name, const char* label)
{
	PropertyRow* row = new PropertyRow(label, label, "");
	lastNode_ = currentNode_;
	enterNode(addRow(row, true));
	return true;
}

void PropertyOArchive::closeBlock()
{
	closeStruct("block");
}

}
// vim:ts=4 sw=4:
