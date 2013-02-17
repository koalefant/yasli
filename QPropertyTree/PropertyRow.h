/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#if __GNUC__ < 4 || __GNUC_MINOR__ < 7
// GCC got support for override keyword in 4.8
# define override
#endif

#include <typeinfo>
#include <algorithm>
#include "yasli/Serializer.h"
#include "yasli/StringList.h"
#include "yasli/Pointers.h"
#include "yasli/Strings.h"
#include "Factory.h"
#include "ConstStringList.h"

#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QRect>

class QWidget;
class QFont;
class QPainter;
class QMenu;
class QKeyEvent;

using std::vector;
class QPropertyTree;
class PropertyRow;
class PropertyTreeModel;
class PopupMenuItem;
struct PropertyDrawContext;

enum ScanResult {
	SCAN_FINISHED, 
	SCAN_CHILDREN, 
	SCAN_SIBLINGS, 
	SCAN_CHILDREN_SIBLINGS,
};

struct PropertyRowMenuHandler : QObject
{
	Q_OBJECT
public:

	virtual ~PropertyRowMenuHandler() {}

};

class PropertyRowWidget : public QObject
{
	Q_OBJECT
public:
	PropertyRowWidget(PropertyRow* row, QPropertyTree* tree);
	virtual ~PropertyRowWidget();
	virtual QWidget* actualWidget() { return 0; }
	virtual void commit() = 0;
	PropertyRow* row() { return row_; }
	PropertyTreeModel* model() { return model_; }
protected:
	PropertyRow* row_;
	QPropertyTree* tree_;
	PropertyTreeModel* model_;
};

class PropertyTreeTransaction;

class PropertyRow : public yasli::RefCounter
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
	PropertyRow(const char* name, const char* nameAlt, const char* typeName);
	PropertyRow(const char* name, const char* nameAlt, const yasli::Serializer& ser);
	virtual ~PropertyRow();

	bool selected() const{ return selected_; }
	void setSelected(bool selected) { selected_ = selected; }
	bool expanded() const{ return expanded_; }
	void _setExpanded(bool expanded); // use QPropertyTree::expandRow
	void setExpandedRecursive(QPropertyTree* tree, bool expanded);

	void setMatchFilter(bool matchFilter) { matchFilter_ = matchFilter; }
	bool matchFilter() const { return matchFilter_; }

	void setBelongsToFilteredRow(bool belongs) { belongsToFilteredRow_ = belongs; }
	bool belongsToFilteredRow() const { return belongsToFilteredRow_; }

	bool visible(const QPropertyTree* tree) const;
	bool hasVisibleChildren(const QPropertyTree* tree, bool internalCall = false) const;

	const PropertyRow* hit(const QPropertyTree* tree, QPoint point) const;
	PropertyRow* hit(const QPropertyTree* tree, QPoint point);
	PropertyRow* parent() { return parent_; }
	const PropertyRow* parent() const{ return parent_; }
	void setParent(PropertyRow* row) { parent_ = row; }
	bool isRoot() const { return !parent_; }
	int level() const;

	PropertyRow* add(PropertyRow* row, PropertyRow* after = 0);
	PropertyRow* addBefore(PropertyRow* row, PropertyRow* before);

	template<class Op> bool scanChildren(Op& op);
	template<class Op> bool scanChildren(Op& op, QPropertyTree* tree);
	template<class Op> bool scanChildrenReverse(Op& op, QPropertyTree* tree);
	template<class Op> bool scanChildrenBottomUp(Op& op, QPropertyTree* tree);

	PropertyRow* childByIndex(int index);
	const PropertyRow* childByIndex(int index) const;
	int childIndex(PropertyRow* row);
	bool isChildOf(const PropertyRow* row) const;

	bool empty() const{ return children_.empty(); }
	iterator find(PropertyRow* row) { return std::find(children_.begin(), children_.end(), row); }
	PropertyRow* findByAddress(void* addr);
	iterator begin() { return children_.begin(); }
	iterator end() { return children_.end(); }
	const_iterator begin() const{ return children_.begin(); }
	const_iterator end() const{ return children_.end(); }
	std::size_t count() const{ return children_.size(); }
	PropertyRow* front() { return children_.front(); }
	PropertyRow* back() { return children_.back(); }
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
	void setLabel(const char* label) { label_ = label ? label : ""; setLabelChanged(); }
	void setLabelChanged();
	void updateLabel(const QPropertyTree* tree);
	void parseControlCodes(const char* label, bool updateLabel);
	const char* typeName() const{ return typeName_; }
	const char* typeNameForFilter() const;
	void setTypeName(const char* typeName) { typeName_ = typeName; }
	yasli::string rowText(const QPropertyTree* tree) const;

	PropertyRow* findSelected();
	PropertyRow* find(const char* name, const char* nameAlt, const char* typeName, bool skipUpdated = false);
	const PropertyRow* find(const char* name, const char* nameAlt, const char* typeName, bool skipUpdated = false) const;
	void intersect(const PropertyRow* row);

	int verticalIndex(QPropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByVerticalIndex(QPropertyTree* tree, int index);
	int horizontalIndex(QPropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByHorizontalIndex(QPropertyTree* tree, int index);

	template<class T>
	bool assignTo(T& object){
		return assignTo(reinterpret_cast<void*>(&object), sizeof(T));
	}
	virtual bool assignTo(void* object, size_t size) { return false; }
	virtual yasli::string valueAsString() const;
	virtual yasli::wstring valueAsWString() const;

	int height() const{ return size_.y(); }

	virtual int widgetSizeMin() const { return userWidgetSize() >= 0 ? userWidgetSize() : 0; } 
	virtual int floorHeight() const{ return 0; }

	void calculateMinimalSize(const QPropertyTree* tree);
	void setTextSize(float multiplier);
	void calculateTotalSizes(int* minTextSize);
	void adjustRect(const QPropertyTree* tree, const QRect& rect, QPoint pos, int& totalHeight, int& extraSize);

	virtual bool isWidgetFixed() const{ return userFixedWidget_ || widgetPlacement() != WIDGET_VALUE; }

	virtual WidgetPlacement widgetPlacement() const{ return WIDGET_NONE; }

	const QRect rect() const{ return QRect(pos_.x(), pos_.y(), size_.x(), size_.y()); }
	const QRect textRect() const{ return QRect(textPos_, pos_.y(), textSize_, ROW_DEFAULT_HEIGHT); }
	const QRect widgetRect() const{ return QRect(widgetPos_, pos_.y(), widgetSize_, ROW_DEFAULT_HEIGHT); }
	const QRect plusRect() const{ return QRect(pos_.x(), pos_.y(), plusSize_, ROW_DEFAULT_HEIGHT); }
	const QRect floorRect() const { return QRect(textPos_, pos_.y() + ROW_DEFAULT_HEIGHT, size_.x(), size_.y() - ROW_DEFAULT_HEIGHT); }
	void adjustHoveredRect(QRect& hoveredRect);
	const QFont* rowFont(const QPropertyTree* tree) const;

	void drawRow(QPainter& painter, const QPropertyTree* tree);
	void drawPlus(QPainter& p, const QPropertyTree* tree, const QRect& rect, bool expanded, bool selected, bool grayed) const;
	void drawStaticText(QPainter& p, const QRect& widgetRect);

	virtual void redraw(const PropertyDrawContext& context);
	virtual PropertyRowWidget* createWidget(QPropertyTree* tree) { return 0; }

	virtual bool isContainer() const{ return false; }
	virtual bool isPointer() const{ return false; }
	virtual bool isObject() const{ return false; }

	virtual bool isLeaf() const{ return false; }
	virtual bool isStatic() const{ return pulledContainer_ == 0; }
	virtual bool isSelectable() const{ return true; }
	virtual bool activateOnAdd() const{ return false; }

	bool canBeToggled(const QPropertyTree* tree) const;
	bool canBeDragged() const;
	bool canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const QPropertyTree* tree) const;
	void dropInto(PropertyRow* parentRow, PropertyRow* cursorRow, QPropertyTree* tree, bool before);

	virtual bool onActivate(QPropertyTree* tree, bool force);
	virtual bool onKeyDown(QPropertyTree* tree, const QKeyEvent* ev);
	virtual bool onMouseDown(QPropertyTree* tree, QPoint point, bool& changed) { return false; }
	virtual void onMouseMove(QPropertyTree* tree, QPoint point) {}
	virtual void onMouseUp(QPropertyTree* tree, QPoint point) {}
	virtual bool onContextMenu(QMenu &menu, QPropertyTree* tree);

	bool isFullRow(const QPropertyTree* tree) const;

	// User states.
	// Assigned using control codes (characters in the beginning of label)
	// fixed widget doesn't expand automatically to occupy all available place
	bool userFixedWidget() const{ return userFixedWidget_; }
	bool userFullRow() const { return userFullRow_; }
	bool userReadOnly() const { return userReadOnly_; }
	bool userReadOnlyRecurse() const { return userReadOnlyRecurse_; }
	int userWidgetSize() const{ return userWidgetSize_; }

	void setUpdated(bool updated) { updated_ = updated; }
	bool updated() const { return updated_; }

	// multiValue is used to edit properties of multiple objects simulateneously
	bool multiValue() const { return multiValue_; }
	void setMultiValue(bool multiValue) { multiValue_ = multiValue; }

	// pulledRow - is the one that is pulled up to the parents row
	// (created with ^ in the beginning of label)
	bool pulledUp() const { return pulledUp_; }
	bool pulledBefore() const { return pulledBefore_; }
	bool hasPulled() const { return hasPulled_; }
	bool pulledSelected() const;
	PropertyRow* nonPulledParent();
	void setPulledContainer(PropertyRow* container){ pulledContainer_ = container; }
	PropertyRow* pulledContainer() { return pulledContainer_; }
	const PropertyRow* pulledContainer() const{ return pulledContainer_; }

	PropertyRow* cloneChildren(PropertyRow* result, const PropertyRow* source) const;
	virtual PropertyRow* clone() const {
		return cloneChildren(serializer_ ? new PropertyRow(name_, label_, serializer_) : new PropertyRow(name_, label_, typeName_), this);
	}

	const wchar_t* digest() const{ return digest_.c_str(); }
	virtual yasli::wstring digestValue() const{ return valueAsWString(); }
	virtual void digestReset(const QPropertyTree* tree);
	void digestAppend(const wchar_t* text);

	yasli::Serializer serializer() const{ return serializer_; }
    void setSerializer(const yasli::Serializer& ser) { serializer_ = ser; }
	virtual void serializeValue(yasli::Archive& ar) {}
	void serialize(yasli::Archive& ar);

	static void setConstStrings(ConstStringList* constStrings){ constStrings_ = constStrings; }

protected:
	void init(const char* name, const char* nameAlt, const char* typeName);

	const char* name_;
	const char* label_;
	const char* labelUndecorated_;
	const char* typeName_;
	yasli::Serializer serializer_;
	yasli::wstring digest_;
	PropertyRow* parent_;
	Rows children_;

	bool visible_ : 1;
	bool matchFilter_ : 1;
	bool belongsToFilteredRow_ : 1;
	bool expanded_ : 1;
	bool selected_ : 1;
	bool updated_ : 1;
	bool labelChanged_ : 1;
	bool userReadOnly_ : 1;
	bool userReadOnlyRecurse_ : 1;
	bool userFixedWidget_ : 1;
	bool userFullRow_ : 1;
	bool pulledUp_ : 1;
	bool pulledBefore_ : 1;
	bool hasPulled_ : 1;
	bool multiValue_ : 1;
	// number of pulled childrens
	char freePulledChildren_;

	// do we really need QPoint here? 
	QPoint pos_;
	QPoint size_;
	short int minimalWidth_;
	short int plusSize_;
	short int textPos_;
	short int textSizeInitial_;
	short int textSize_;
	short int digestPos_;
	short int widgetPos_; // widget == icon!
	short int widgetSize_;
	short int userWidgetSize_;
	unsigned textHash_;

	yasli::SharedPtr<PropertyRow> pulledContainer_;

	static ConstStringList* constStrings_;
};

typedef vector<yasli::SharedPtr<PropertyRow> > PropertyRows;

template<bool value>
struct StaticBool{
	enum { Value = value };
};

class PropertyRowArg{
public:
	PropertyRowArg(void* object, size_t size, const char* name, const char* nameAlt, const char* typeName){
		object_ = object;
		size_ = size;
		name_ = name;
		nameAlt_ = nameAlt;
		typeName_ = typeName;
	}

	template<class T>
	void construct(T** row)
	{
		*row = (T*)createRow<T>(StaticBool<true>());
	}

	template<class Derived>
	static PropertyRow* createRow(StaticBool<true> custom)
	{
		PropertyRow* result;
		if(object_)
			result = new Derived(object_, size_, name_, nameAlt_, typeName_);
		else
			result = new Derived();
		return result;
	}

	template<class Derived>
	static PropertyRow* createRow(StaticBool<false> custom)
	{
		PropertyRow* result = new Derived();
		return result;
	}

	template<class Derived>
	static PropertyRow* createArg(){ 
		return createRow<Derived>(StaticBool<Derived::Custom>());
	}

protected:
	static void* object_;
	static size_t size_;
	static const char* name_;
	static const char* nameAlt_;
	static const char* typeName_;
};

typedef Factory<yasli::string, PropertyRow, PropertyRowArg> PropertyRowFactory;

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
bool PropertyRow::scanChildren(Op& op, QPropertyTree* tree)
{
	Rows::iterator it;
	for(it = children_.begin(); it != children_.end(); ++it){
		ScanResult result = op(*it, tree);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!(*it)->scanChildren(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenReverse(Op& op, QPropertyTree* tree)
{
	for(Rows::reverse_iterator it = children_.rbegin(); it != children_.rend(); ++it){
		ScanResult result = op(*it, tree);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!(*it)->scanChildrenReverse(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenBottomUp(Op& op, QPropertyTree* tree)
{
	for(Rows::iterator it = children_.begin(); it != children_.end(); ++it)
	{
		if(!(*it)->scanChildrenBottomUp(op, tree))
			return false;
		ScanResult result = op(*it, tree);
		if(result == SCAN_FINISHED)
			return false;
	}
	return true;
}


#define REGISTER_PROPERTY_ROW(DataType, RowType) \
	REGISTER_IN_FACTORY(PropertyRowFactory, yasli::TypeID::get<DataType>().name(), RowType); \
	YASLI_CLASS(PropertyRow, RowType, #DataType);


