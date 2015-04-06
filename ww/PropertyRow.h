/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <typeinfo>
#include <algorithm>
#include "ww/Widget.h"
#include "yasli/Serializer.h"
#include "yasli/StringList.h"
#include "ww/Factory.h"
#include "ww/ConstStringList.h"
#include "ww/Win32/Types.h"

namespace Gdiplus{
	class Graphics;
	class Rect;
	class Font;
}

namespace ww{
using std::vector;
class PropertyTree;
class PropertyRow;
class PropertyTreeModel;
class PopupMenuItem;
struct PropertyDrawContext;
struct KeyPress;

enum ScanResult {
	SCAN_FINISHED, 
	SCAN_CHILDREN, 
	SCAN_SIBLINGS, 
	SCAN_CHILDREN_SIBLINGS,
};





class PropertyRowWidget : public PolyRefCounter{
public:
	PropertyRowWidget(PropertyRow* row, PropertyTreeModel* model)
	: row_(row)
	, model_(model)
	{
	}
	virtual ~PropertyRowWidget();
	virtual Widget* actualWidget() { return 0; }
	virtual void commit() = 0;
	PropertyRow* row() { return row_; }
	PropertyTreeModel* model() { return model_; }
protected:
	PropertyRow* row_;
	PropertyTreeModel* model_;
};

class PropertyTreeTransaction;

class PropertyRow : public RefCounter
{
public:
	enum WidgetPlacement {
		WIDGET_NONE,
		WIDGET_ICON,
		WIDGET_AFTER_NAME,
		WIDGET_VALUE
	};

	static const int ROW_DEFAULT_HEIGHT = 21; 
	static const int ICON_SIZE = 21; 
	static const int TEXT_SIZE_MIN = 30; 
	static const int WIDGET_SIZE_MIN = 30; 
	static const bool Custom = true;

	typedef std::vector< yasli::SharedPtr<PropertyRow> > Rows;
	typedef Rows::iterator iterator;
	typedef Rows::const_iterator const_iterator;

	PropertyRow();
	virtual ~PropertyRow();

	void setNames(const char* name, const char* label, const char* typeName);

	bool selected() const{ return selected_; }
	void setSelected(bool selected) { selected_ = selected; }
	bool expanded() const{ return expanded_; }
	void _setExpanded(bool expanded); // use ropertyTree::expandRow
	void setExpandedRecursive(PropertyTree* tree, bool expanded);

	void setMatchFilter(bool matchFilter) { matchFilter_ = matchFilter; }
	bool matchFilter() const { return matchFilter_; }

	void setBelongsToFilteredRow(bool belongs) { belongsToFilteredRow_ = belongs; }
	bool belongsToFilteredRow() const { return belongsToFilteredRow_; }

	bool visible(const PropertyTree* tree) const;
	bool hasVisibleChildren(const PropertyTree* tree, bool internalCall = false) const;

	const PropertyRow* hit(const PropertyTree* tree, Vect2 point) const;
	PropertyRow* hit(const PropertyTree* tree, Vect2 point);
	PropertyRow* parent() { return parent_; }
	const PropertyRow* parent() const{ return parent_; }
	void setParent(PropertyRow* row) { parent_ = row; }
	bool isRoot() const { return !parent_; }
	int level() const;

	void add(PropertyRow* row);
	void addAfter(PropertyRow* row, PropertyRow* after);
	void addBefore(PropertyRow* row, PropertyRow* before);

	template<class Op> bool scanChildren(Op& op);
	template<class Op> bool scanChildren(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenReverse(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenBottomUp(Op& op, PropertyTree* tree);

	PropertyRow* childByIndex(int index);
	const PropertyRow* childByIndex(int index) const;
	int childIndex(PropertyRow* row);
	bool isChildOf(const PropertyRow* row) const;
	
	bool empty() const{ return children_.empty(); }
	iterator find(PropertyRow* row) { return std::find(children_.begin(), children_.end(), row); }
	PropertyRow* findFromIndex(int* outIndex, const char* name, const char* typeName, int startIndex) const;
    PropertyRow* findByAddress(void* addr);
	iterator begin() { return children_.begin(); }
	iterator end() { return children_.end(); }
	const_iterator begin() const{ return children_.begin(); }
	const_iterator end() const{ return children_.end(); }
	std::size_t count() const{ return children_.size(); }
	iterator erase(iterator it){ return children_.erase(it); }
	void clear(){ children_.clear(); }
	void erase(PropertyRow* row);
	void swapChildren(PropertyRow* row);
	
    void assignRowState(const PropertyRow& row, bool recurse);
	void assignRowProperties(PropertyRow* row);
	void replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, bool preserveChildren = true);

	const char* name() const{ return name_; }
	void setName(const char* name) { name_ = name; }
	const char* label() const { return label_; }
	const char* labelUndecorated() const { return labelUndecorated_; }
	void setLabel(const char* label);
	void setLabelChanged();
	void setLayoutChanged();
	void setLabelChangedToChildren();
	void setLayoutChangedToChildren();
	void updateLabel(const PropertyTree* tree, int index);
	void updateTextSizeInitial(const PropertyTree* tree, int index);
	virtual void labelChanged() {}
	void parseControlCodes(const char* label, bool changeLabel);
	const char* typeName() const{ return typeName_; }
	const char* typeNameForFilter() const;
	void setTypeName(const char* typeName) { typeName_ = typeName; }
	const char* rowText(char (&containerLabelBuffer)[16], const PropertyTree* tree, int rowIndex) const;

	PropertyRow* findSelected();
	PropertyRow* find(const char* name, const char* nameAlt, const char* typeName);
	const PropertyRow* find(const char* name, const char* nameAlt, const char* typeName) const;
	void intersect(const PropertyRow* row);

	int verticalIndex(PropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByVerticalIndex(PropertyTree* tree, int index);
	int horizontalIndex(PropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByHorizontalIndex(PropertyTree* tree, int index);

	template<class T>
	bool assignTo(T& object){
		return assignTo(reinterpret_cast<void*>(&object), sizeof(T));
	}
	virtual bool assignTo(void* object, size_t size) { return false; }
	virtual void setValue(const yasli::Serializer& ser) { serializer_ = ser; }
	virtual string valueAsString() const;
	virtual wstring valueAsWString() const;

	int height() const{ return size_.y; }

	virtual int widgetSizeMin() const { return userWidgetSize() >= 0 ? userWidgetSize() : 0; } 
	virtual int floorHeight() const{ return 0; }

	void calcPulledRows(int* minTextSize, int* freePulledChildren, int* minimalWidth, const PropertyTree* tree, int index);
    void calculateMinimalSize(const PropertyTree* tree, int posX, bool force, int* _extraSize, int index);
	void setTextSize(const PropertyTree* tree, int rowIndex, float multiplier);
	void calculateTotalSizes(int* minTextSize);
	void adjustVerticalPosition(const PropertyTree* tree, int& totalHeight);

	virtual bool isWidgetFixed() const{ return userFixedWidget_ || widgetPlacement() != WIDGET_VALUE; }

	virtual WidgetPlacement widgetPlacement() const{ return WIDGET_NONE; }

	int posY() const { return pos_.y + offsetY_; }
	Rect rect() const{ return Rect(pos_.x, posY(), pos_.x + size_.x, posY() + size_.y); }
	Rect textRect() const{ return Rect(textPos_, posY(), textPos_ + textSize_, posY() + ROW_DEFAULT_HEIGHT); }
    Rect widgetRect() const{ return Rect(widgetPos_, posY(), widgetPos_ + widgetSize_, posY() + ROW_DEFAULT_HEIGHT); }
    Rect plusRect() const{ return Rect(pos_.x, posY(), pos_.x + plusSize_, posY() + ROW_DEFAULT_HEIGHT); }
	Rect floorRect() const { return Rect(textPos_, posY() + ROW_DEFAULT_HEIGHT, pos_.x + size_.x, posY() + size_.y); }
	Gdiplus::Font* rowFont(const PropertyTree* tree) const;
	
	void drawRow(HDC dc, const PropertyTree* tree, int index);
    void drawPlus(Gdiplus::Graphics* gr, const Rect& rect, bool expanded, bool selected, bool grayed) const;

	virtual void redraw(const PropertyDrawContext& context);
	virtual PropertyRowWidget* createWidget(PropertyTree* tree) { return 0; }
	
	virtual bool isContainer() const{ return false; }
	virtual bool isPointer() const{ return false; }
	virtual bool isObject() const{ return false; }

	virtual bool isLeaf() const{ return false; }
	virtual bool isStatic() const{ return pulledContainer_ == 0; }
	virtual bool isSelectable() const{ return true; }
	virtual bool activateOnAdd() const{ return false; }
	
	bool canBeToggled(const PropertyTree* tree) const;
	bool canBeDragged() const;
	bool canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const PropertyTree* tree) const;
	void dropInto(PropertyRow* parentRow, PropertyRow* cursorRow, PropertyTree* tree, bool before);

	virtual bool onActivate(PropertyTree* tree, bool force);
	virtual bool onActivateRelease(PropertyTree* tree) { return false; }
	virtual bool onKeyDown(PropertyTree* tree, KeyPress key);
	virtual bool onMouseDown(PropertyTree* tree, Vect2 point, bool& changed) { return false; }
	virtual void onMouseMove(PropertyTree* tree, Vect2 point) {}
	virtual void onMouseUp(PropertyTree* tree, Vect2 point) {}
	virtual bool onContextMenu(PopupMenuItem &root, PropertyTree* tree);

	bool isFullRow(const PropertyTree* tree) const;

	// User states.
	// Assigned using control codes (characters in the beginning of label)
	// fixed widget doesn't expand automatically to occupy all available place
	bool userFixedWidget() const{ return userFixedWidget_; }
	bool userFullRow() const { return userFullRow_; }
	bool userReadOnly() const { return userReadOnly_; }
	bool userReadOnlyRecurse() const { return userReadOnlyRecurse_; }
	int userWidgetSize() const{ return userWidgetSize_; }

	// multiValue is used to edit properties of multiple objects simulateneously
	bool multiValue() const { return multiValue_; }
	void setMultiValue(bool multiValue) { multiValue_ = multiValue; }

	// pulledRow - is the one that is pulled up to the parents row
	// (created with ^ in the beginning of label)
	bool pulledUp() const { return pulledUp_; }
   	bool pulledBefore() const { return pulledBefore_; } // добавление перед по символу '`', дополнительный к pulledUp
	bool hasPulled() const { return hasPulled_; }
	bool pulledSelected() const;
	PropertyRow* nonPulledParent();
	void setPulledContainer(PropertyRow* container){ pulledContainer_ = container; }
	PropertyRow* pulledContainer() { return pulledContainer_; }
	const PropertyRow* pulledContainer() const{ return pulledContainer_; }

	PropertyRow* cloneChildren(PropertyRow* result, const PropertyRow* source) const;
	virtual PropertyRow* clone() const {
		PropertyRow* result = new PropertyRow();
		result->setNames(name_, label_, typeName_);
		result->setValue(serializer_);
		return cloneChildren(result, this);
	}

	Serializer serializer() const{ return serializer_; }
    void setSerializer(const Serializer& ser) { serializer_ = ser; }
	virtual void serializeValue(Archive& ar) {}
	void serialize(Archive& ar);

	static void setConstStrings(ConstStringList* constStrings){ constStrings_ = constStrings; }
	static void setOffsetY(int offset) { offsetY_ = offset; }

protected:
	void init(const char* name, const char* nameAlt, const char* typeName);

	const char* name_;
	const char* label_;
	const char* labelUndecorated_;
	const char* typeName_;
    Serializer serializer_;
	PropertyRow* parent_;
	Rows children_;

	bool visible_ : 1;
	bool matchFilter_ : 1;
	bool belongsToFilteredRow_ : 1;
	bool expanded_ : 1;
	bool selected_ : 1;
	bool labelChanged_ : 1;
	bool layoutChanged_ : 1;
	bool userReadOnly_ : 1;
	bool userReadOnlyRecurse_ : 1;
	bool userFixedWidget_ : 1;
	bool userFullRow_ : 1;
	bool pulledUp_ : 1;
	bool pulledBefore_ : 1;
	bool hasPulled_ : 1;
	bool multiValue_ : 1;

	// do we really need Vect2s here? 
	Vect2 pos_;
	Vect2 size_;
    short int plusSize_;
	short int textPos_;
	short int textSizeInitial_;
	short int textSize_;
	short int widgetPos_; // widget == icon!
	short int widgetSize_;
	short int userWidgetSize_;
	unsigned textHash_;

	yasli::SharedPtr<PropertyRow> pulledContainer_;

	static ConstStringList* constStrings_;
	static int offsetY_;
	friend class PropertyOArchive;
	friend class PropertyIArchive;
};

typedef vector<yasli::SharedPtr<PropertyRow> > PropertyRows;


template<bool value>
struct StaticBool{
	enum { Value = value };
};

class PropertyRowArg{
public:
	PropertyRowArg(const char* name, const char* label, const char* typeName){
		name_ = name;
		label_ = label;
		typeName_ = typeName;
	}

	template<class T>
	void construct(T** row)
	{
		*row = createRow<T>();
	}

	template<class Derived>
	static Derived* createRow()
	{
		Derived* result = new Derived(name_, label_, typeName_);
		return result;
	}

	template<class Derived>
	static PropertyRow* createArg(){ 
		return createRow<Derived>();
	}

protected:
	static const char* name_;
	static const char* label_;
	static const char* typeName_;
};

struct LessStrCmp
{
	bool operator()(const char* a, const char* b) const {
		return strcmp(a, b) < 0;
	}
};

typedef Factory<const char*, PropertyRow, Constructor0<PropertyRow>, LessStrCmp> PropertyRowFactory;

template<class Op>
bool PropertyRow::scanChildren(Op& op)
{
	Rows::iterator it;

	for(it = children_.begin(); it != children_.end(); ++it){
		ScanResult result = op(*it);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!(*it)->scanChildren(op))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildren(Op& op, PropertyTree* tree)
{
	int numChildren = children_.size();
	for(int index = 0; index < numChildren; ++index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!child->scanChildren(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenReverse(Op& op, PropertyTree* tree)
{
	int numChildren = children_.size();
	for(int index = numChildren - 1; index >= 0; --index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!child->scanChildrenReverse(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenBottomUp(Op& op, PropertyTree* tree)
{
	size_t numChildren = children_.size();
	for(size_t i = 0; i < numChildren; ++i)
	{
		PropertyRow* child = children_[i];
		if(!child->scanChildrenBottomUp(op, tree))
			return false;
		ScanResult result = op(child, tree);
		if(result == SCAN_FINISHED)
			return false;
	}
	return true;
}


}

#define REGISTER_PROPERTY_ROW(DataType, RowType) \
	REGISTER_IN_FACTORY(PropertyRowFactory, TypeID::get<DataType>().name(), RowType); \
	YASLI_CLASS(PropertyRow, RowType, #DataType);


