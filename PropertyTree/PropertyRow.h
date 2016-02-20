/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#if !defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
// GCC got support for override keyword in 4.8
# define override
# define final
#endif

#include <typeinfo>
#include <algorithm>
#include "yasli/Serializer.h"
#include "yasli/StringList.h"
#include "yasli/Pointers.h"
#include "yasli/Config.h"
#include "Factory.h"
#include "ConstStringList.h"
#include "IDrawContext.h"
#include "IUIFacade.h"
#include "Rect.h"
#include "Layout.h"
#include "sigslot.h"

#ifdef _MSC_VER
#pragma warning(disable: 4264) // no override available for virtual member function from base 'PropertyRow'; function is hidden
#endif

class QPainter;
namespace property_tree { 
struct KeyEvent; 
class IMenu;
class InplaceWidget;
}
using property_tree::KeyEvent;

using std::vector;
class PropertyTree;
class PropertyRow;
class PropertyTreeModel;

enum ScanResult {
	SCAN_FINISHED, 
	SCAN_CHILDREN, 
	SCAN_SIBLINGS, 
	SCAN_CHILDREN_SIBLINGS,
};

struct PropertyRowMenuHandler : sigslot::has_slots
{
	virtual ~PropertyRowMenuHandler() {}
};

struct PropertyActivationEvent
{
	enum Reason
	{
		REASON_PRESS,
		REASON_RELEASE,
		REASON_DOUBLECLICK,
		REASON_KEYBOARD,
		REASON_NEW_ELEMENT,
		REASON_CONTEXT_MENU
	};

	PropertyTree* tree;
	Reason reason;
	bool rename;
	Point clickPoint;

	PropertyActivationEvent()
	: rename(false)
	, clickPoint(0, 0)
	, tree(0)
	, reason(REASON_PRESS)
	{
	}
};

struct PropertyDragEvent
{
	::PropertyTree* tree;
	Point pos;
	Point start;
	Point lastDelta;
	Point totalDelta;
};

struct PropertyHoverInfo
{
	property_tree::Cursor cursor;
	yasli::string toolTip;

	PropertyHoverInfo()
	: cursor(CURSOR_ARROW)
	{
	}
};

enum DragCheckBegin {
	DRAG_CHECK_IGNORE,
	DRAG_CHECK_SET,
	DRAG_CHECK_UNSET
};

class PropertyTreeTransaction;
class PropertyRowStruct;

class PROPERTY_TREE_API PropertyRow : public yasli::RefCounter
{
public:
	enum WidgetPlacement {
		WIDGET_NONE,
		WIDGET_ICON,
		WIDGET_AFTER_NAME,
		WIDGET_VALUE,
		WIDGET_AFTER_INLINED,
		WIDGET_INSTEAD_OF_TEXT
	};


	PropertyRow(bool isStruct = false, bool isContainer = false);
	virtual ~PropertyRow();

	void setNames(const char* name, const char* label, const char* typeName);

	bool selected() const{ return selected_; }
	void setSelected(bool selected) { selected_ = selected; }
	bool expanded() const{ return expanded_ || inlined_ || inlinedBefore_; }
	void _setExpanded(bool expanded); // use PropertyTree::expandRow
	void setExpandedRecursive(PropertyTree* tree, bool expanded);

	void setMatchFilter(bool matchFilter) { matchFilter_ = matchFilter; }
	bool matchFilter() const { return matchFilter_; }

	void setBelongsToFilteredRow(bool belongs) { belongsToFilteredRow_ = belongs; }
	bool belongsToFilteredRow() const { return belongsToFilteredRow_; }

	bool visible(const PropertyTree* tree) const;
	bool hasVisibleChildren(const PropertyTree* tree, bool internalCall = false) const;

	PropertyRowStruct* parent() const { return parent_; }
	void setParent(PropertyRowStruct* row) { parent_ = row; }
	bool isRoot() const { return !parent_; }
	int level() const;

	PropertyRow* childByIndex(int index);
	const PropertyRow* childByIndex(int index) const;
	int childIndex(const PropertyRow* row) const;
	bool isChildOf(const PropertyRow* row) const;

	PropertyRow* findFromIndex(int* outIndex, const char* name, const char* typeName, int startIndex) const;
	PropertyRow* findByAddress(const void* handle);
	virtual const void* searchHandle() const;
	virtual yasli::TypeID searchType() const { return serializer().type(); }
	size_t count() const;
	virtual void swapChildren(PropertyRow* row, PropertyTreeModel* model) {}

	void assignRowState(const PropertyRow& row, bool recurse);
	void assignRowProperties(PropertyRow* row);

	const char* name() const{ return name_; }
	void setName(const char* name) { name_ = name; }
	const char* label() const { return label_; }
	const char* labelUndecorated() const { return label_ + controlCharacterCount_; }
	void setLabel(const char* label);
	void setLabelChanged();

	const char* tooltip() const { return tooltip_; }
	void setTooltip(const char* tooltip);

	bool setValidatorEntry(int index, int count);
	int validatorCount() const{ return validatorCount_; }
	int validatorIndex() const{ return validatorIndex_; }
	bool validatorHasWarnings() const{ return validatorHasWarnings_; }
	bool validatorHasErrors() const{ return validatorHasErrors_; }
	void resetValidatorIcons();
	void addValidatorIcons(bool hasWarnings, bool hasErrors);

	void setLayoutChanged();
	void setLabelChangedToChildren();
	void setLayoutChangedToChildren();
	void setHideChildren(bool hideChildren) { hideChildren_ = hideChildren; }
	bool hideChildren() const { return hideChildren_; }
	void updateLabel(const PropertyTree* tree, int index, bool parentHidesNonInlineChildren);
	virtual void labelChanged() {}
	void parseControlCodes(const PropertyTree* tree, const char* label, bool changeLabel);
	const char* typeName() const{ return typeName_; }
	virtual yasli::string typeNameForFilter(PropertyTree* tree) const;
	void setTypeName(const char* typeName) { YASLI_ASSERT(strlen(typeName)); typeName_ = typeName; }
	const char* rowText(char (&containerLabelBuffer)[16], const PropertyTree* tree, int rowIndex) const;

	PropertyRow* find(const char* name, const char* nameAlt, const char* typeName);
	const PropertyRow* find(const char* name, const char* nameAlt, const char* typeName) const;
	void intersect(const PropertyRow* row);

	virtual bool assignToPrimitive(void* object, size_t size) const{ return false; }
	virtual bool assignTo(const yasli::Serializer& ser) const{ return false; }
	virtual bool assignToByPointer(void* instance, const yasli::TypeID& type) const{ return assignTo(yasli::Serializer(type, instance, type.sizeOf(), 0)); }
	virtual void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) {}
	virtual void handleChildrenChange() {}
	virtual yasli::string valueAsString() const;
	virtual yasli::wstring valueAsWString() const;

	virtual int widgetSizeMin(const PropertyTree*) const { return userWidgetSize() >= 0 ? userWidgetSize() : 0; } 

	int textSizeInitial() const { return textSizeInitial_; }
	void updateTextSize_r(const PropertyTree* tree, int index);

	virtual bool isWidgetFixed() const{ return userFixedWidget_ || (widgetPlacement() != WIDGET_VALUE && widgetPlacement() != WIDGET_INSTEAD_OF_TEXT); }
	virtual WidgetPlacement widgetPlacement() const{ return WIDGET_NONE; }

	Rect rect(const PropertyTree* tree) const;
	Rect textRect(const PropertyTree* tree) const;
	Rect childrenRect(const PropertyTree* tree) const;
	Rect widgetRect(const PropertyTree* tree) const;
	Rect plusRect(const PropertyTree* tree) const;
	Rect validatorRect(const PropertyTree* tree) const;
	Rect validatorWarningIconRect(const PropertyTree* tree) const;
	Rect validatorErrorIconRect(const PropertyTree* tree) const;
	property_tree::Font rowFont(const PropertyTree* tree) const;

	void drawElement(IDrawContext& x, property_tree::RowPart part, const property_tree::Rect& rect, int partSubindex);

	virtual void redraw(IDrawContext& context);
	virtual property_tree::InplaceWidget* createWidget(PropertyTree* tree) { return 0; }

	bool isContainer() const{ return isContainer_; }
	virtual bool isPointer() const{ return false; }
	virtual bool isObject() const{ return false; }

	virtual bool isLeaf() const{ return false; }
	virtual void closeNonLeaf(const yasli::Serializer& ser, yasli::Archive& ar) {}
	virtual bool isStatic() const{ return inlinedContainer() == 0; }
	virtual bool isSelectable() const{ return (!userReadOnly() && !userReadOnlyRecurse()) || (!inlined() && !inlinedBefore()); }
	virtual bool activateOnAdd() const{ return false; }
	virtual bool inlineInShortArrays() const{ return false; }

	bool canBeToggled(const PropertyTree* tree) const;
	bool canBeDragged() const;
	bool canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const PropertyTree* tree) const;
	void dropInto(PropertyRowStruct* parentRow, PropertyRow* cursorRow, PropertyTree* tree, bool before);
	virtual bool getHoverInfo(PropertyHoverInfo* hit, const Point& cursorPos, const PropertyTree* tree) const { 
		hit->toolTip = tooltip_;
		return true; 
	}

	virtual bool onActivate(const PropertyActivationEvent& e);
	virtual bool onKeyDown(PropertyTree* tree, const KeyEvent* ev);
	virtual bool onMouseDown(PropertyTree* tree, Point point, bool& changed) { return false; }
	virtual void onMouseDrag(const PropertyDragEvent& e) {}
	virtual void onMouseStill(const PropertyDragEvent& e) {}
	virtual void onMouseUp(PropertyTree* tree, Point point) {}
	// "drag check" allows you to "paint" with the mouse through checkboxes to set all values at once
	virtual DragCheckBegin onMouseDragCheckBegin() { return DRAG_CHECK_IGNORE; }
	virtual bool onMouseDragCheck(PropertyTree* tree, bool value) { return false; }
	virtual bool onContextMenu(IMenu &menu, PropertyTree* tree);

	// User states.
	// Assigned using control codes (characters in the beginning of label)
	// fixed widget doesn't expand automatically to occupy all available place
	bool userFixedWidget() const{ return userFixedWidget_; }
	bool userFullRow() const { return userFullRow_; }
	bool userReadOnly() const { return userReadOnly_; }
	bool userReadOnlyRecurse() const { return userReadOnlyRecurse_; }
	bool userWidgetToContent() const { return userWidgetToContent_; }
	bool userRenamable() const { return userRenamable_; }
	int userWidgetSize() const{ return userWidgetSize_; }

	// multiValue is used to edit properties of multiple objects simulateneously
	bool multiValue() const { return multiValue_; }
	void setMultiValue(bool multiValue) { multiValue_ = multiValue; }

	// pulledRow - is the one that is pulled up to the parents row
	// (created with ^ in the beginning of label)
	bool inlined() const { return inlined_; }
	bool inlinedBefore() const { return inlinedBefore_; }
	bool hasInlinedChildren() const { return hasInlinedChildren_; }
	bool inlinedSelected() const;
	PropertyRow* findNonInlinedParent();
	PropertyRow* inlinedContainer();
	const PropertyRow* inlinedContainer() const;

	yasli::SharedPtr<PropertyRow> clone(ConstStringList* constStrings) const;

	bool isStruct() const { return isStruct_; }
	PropertyRowStruct* asStruct();
	const PropertyRowStruct* asStruct() const;
	yasli::Serializer serializer() const;
	virtual yasli::TypeID typeId() const{ return serializer().type(); }
	void setSerializer(const yasli::Serializer& ser);

	virtual void serializeValue(yasli::Archive& ar) {}
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar);

	void setCallback(yasli::CallbackInterface* callback);
	yasli::CallbackInterface* callback() { return callback_; }

	static void setConstStrings(ConstStringList* constStrings){ constStrings_ = constStrings; }

	int layoutElement() const { return layoutElement_; }
	void setLayoutElement(int layoutElement) { layoutElement_ = layoutElement; }

protected:
	void updateInlineTextSize_r(const PropertyTree* tree, int index);
	void updateTextSizeInitial(const PropertyTree* tree, int index);
	void init(const char* name, const char* nameAlt, const char* typeName);

	const char* name_;
	const char* typeName_;
	const char* label_;
	PropertyRowStruct* parent_;
	yasli::CallbackInterface* callback_;
	const char* tooltip_;

	int layoutElement_;
	unsigned int textHash_;

	short int textSizeInitial_;
	short int userWidgetSize_;
	unsigned short validatorIndex_;	
	unsigned short validatorsHeight_;
	unsigned char validatorCount_;
	unsigned char controlCharacterCount_;
	bool isStruct_ : 1;
	bool isContainer_ : 1;
	bool visible_ : 1;
	bool matchFilter_ : 1;
	bool belongsToFilteredRow_ : 1;
	bool expanded_ : 1;
	bool selected_ : 1;
	bool labelChanged_ : 1;
	bool userReadOnly_ : 1;
	bool userReadOnlyRecurse_ : 1;
	bool userRenamable_ : 1;
	bool userFixedWidget_ : 1;
	bool userFullRow_ : 1;
	bool userHideChildren_ : 1;
	bool userPackCheckboxes_ : 1;
	bool userWidgetToContent_ : 1;
	bool inlined_ : 1;
	bool inlinedBefore_ : 1;
	bool hasInlinedChildren_ : 1;
	bool multiValue_ : 1;
	bool hideChildren_ : 1;
	bool validatorHasErrors_ : 1;
	bool validatorHasWarnings_ : 1;

	static ConstStringList* constStrings_;
	friend class PropertyOArchive;
	friend class PropertyIArchive;
	friend class PropertyRowStruct;
};

class PropertyRowStruct : public PropertyRow
{
public:
	typedef std::vector< yasli::SharedPtr<PropertyRow> > Rows;

	PropertyRowStruct(bool isContainer = false) : PropertyRow(true, isContainer) { children_.reserve(8); }
	~PropertyRowStruct();

	yasli::Serializer serializer() const{ return serializer_; }
    void setSerializer(const yasli::Serializer& ser) { serializer_ = ser; }

	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override { serializer_ = ser; }
	size_t count() const{ return children_.size(); }
	PropertyRow* childByIndex(int index);
	const PropertyRow* childByIndex(int index) const;
	int childIndex(const PropertyRow* row) const;

	bool isStatic() const override { return inlinedContainer_ == 0; }
	void add(PropertyRow* row);
	void addAfter(PropertyRow* row, PropertyRow* after);
	void addBefore(PropertyRow* row, PropertyRow* before);
	void clear(){ children_.clear(); }
	void erase(PropertyRow* row);
	void replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, PropertyTreeModel* model, bool preserveChildren);

	const PropertyRowStruct* inlinedContainer() const{ return inlinedContainer_; }
	void setInlinedContainer(PropertyRowStruct* container){ inlinedContainer_ = container; }
	PropertyRowStruct* inlinedContainer() { return inlinedContainer_; }

	void swapChildren(PropertyRow* row, PropertyTreeModel* model) override;

	template<class Op> bool scanChildren(Op& op);
	template<class Op> bool scanChildren(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenReverse(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenBottomUp(Op& op, PropertyTree* tree);

	void serialize(yasli::Archive& ar);
protected:
	Rows children_;
	yasli::Serializer serializer_;
	yasli::SharedPtr<PropertyRowStruct> inlinedContainer_;
	friend class PropertyOArchive;
	friend class PropertyIArchive;
};

inline unsigned int calculateHash(const char* str, unsigned hash = 5381)
{
	while(*str)
		hash = hash * 33 + (unsigned char)*str++;
	return hash;
}

template<class T>
inline unsigned int calculateHash(const T& t, unsigned hash = 5381)
{
	for (int i = 0; i < sizeof(T); i++)
		hash = hash * 33 + ((unsigned char*)&t)[i];
	return hash;
}

struct RowWidthCache
{
	unsigned int valueHash;
	int width;

	RowWidthCache() : valueHash(0), width(-1) {}
	int getOrUpdate(const PropertyTree* tree, const PropertyRow* rowForValue, int extraSpace, const char* text = 0);
};

typedef vector<yasli::SharedPtr<PropertyRow> > PropertyRows;

template<bool value>
struct StaticBool{
	enum { Value = value };
};

struct LessStrCmpPR
{
	bool operator()(const char* a, const char* b) const {
		return strcmp(a, b) < 0;
	}
};

typedef Factory<const char*, PropertyRow, Constructor0<PropertyRow>, LessStrCmpPR> PropertyRowFactory;

template<class Op>
bool PropertyRowStruct::scanChildren(Op& op)
{
	Rows::iterator it;

	for(it = children_.begin(); it != children_.end(); ++it){
		ScanResult result = op(*it);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if((*it)->isStruct()) {
				PropertyRowStruct* schild = static_cast<PropertyRowStruct*>(it->get());
				if(!schild->scanChildren(op))
					return false;
				if(result == SCAN_CHILDREN)
					return false;
			}
		}
	}
	return true;
}

template<class Op>
bool PropertyRowStruct::scanChildren(Op& op, PropertyTree* tree)
{
	int numChildren = (int)children_.size();
	for(int index = 0; index < numChildren; ++index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if (child->isStruct()) {
				PropertyRowStruct* schild = static_cast<PropertyRowStruct*>(child);
				if(!schild->scanChildren(op, tree))
					return false;
				if(result == SCAN_CHILDREN)
					return false;
			}
		}
	}
	return true;
}

template<class Op>
bool PropertyRowStruct::scanChildrenReverse(Op& op, PropertyTree* tree)
{
	int numChildren = (int)children_.size();
	for(int index = numChildren - 1; index >= 0; --index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if (child->isStruct()) {
				PropertyRowStruct* schild = static_cast<PropertyRowStruct*>(child);
				if(!schild->scanChildrenReverse(op, tree))
					return false;
				if(result == SCAN_CHILDREN)
					return false;
			}
		}
	}
	return true;
}

template<class Op>
bool PropertyRowStruct::scanChildrenBottomUp(Op& op, PropertyTree* tree)
{
	size_t numChildren = children_.size();
	for(size_t i = 0; i < numChildren; ++i)
	{
		PropertyRow* child = children_[i];
		if (child->isStruct())
			if(!static_cast<PropertyRowStruct*>(child)->scanChildrenBottomUp(op, tree))
				return false;
		ScanResult result = op(child, tree);
		if(result == SCAN_FINISHED)
			return false;
	}
	return true;
}

template<class T>
inline void visitPulledRows(PropertyRow* row, T& drawFunc) 
{
	int count = (int)row->count();
	for (int i = 0; i < count; ++i) {
		PropertyRow* child = row->childByIndex(i);
		if (child->inlined() || child->inlinedBefore()) {
			drawFunc(child);
			visitPulledRows(child, drawFunc);
		}
	}
};

YASLI_API PropertyRowFactory& globalPropertyRowFactory();
YASLI_API yasli::ClassFactory<PropertyRow>& globalPropertyRowClassFactory();

struct PropertyRowPtrSerializer : yasli::SharedPtrSerializer<PropertyRow>
{
	PropertyRowPtrSerializer(yasli::SharedPtr<PropertyRow>& ptr) : SharedPtrSerializer(ptr) {}
	yasli::ClassFactory<PropertyRow>* factory() const override { return &globalPropertyRowClassFactory(); }
};

inline bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, yasli::SharedPtr<PropertyRow>& ptr, const char* name, const char* label)
{
	PropertyRowPtrSerializer serializer(ptr);
	return ar(static_cast<yasli::PointerInterface&>(serializer), name, label);
}

#define REGISTER_PROPERTY_ROW(DataType, RowType) \
	REGISTER_IN_FACTORY(PropertyRowFactory, yasli::TypeID::get<DataType>().name(), RowType); \
	YASLI_CLASS_NAME(PropertyRow, RowType, #RowType, #DataType);


