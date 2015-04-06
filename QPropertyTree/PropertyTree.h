#pragma once
#include <memory>
#include "PropertyTreeStyle.h"
#include <vector>
#include <yasli/Archive.h>
#include <yasli/Pointers.h>
#include "Rect.h"
#include "sigslot.h"
using std::vector;

class IMenu;
struct Color;
class PropertyTreeModel;
class PopupMenuItem;
class PropertyTreeModel;
class PropertyRow;
typedef vector<yasli::SharedPtr<PropertyRow> > PropertyRows;
class InplaceWidget;
class PropertyTreeOperator;
struct PropertyRowMenuHandler;
class Entry;
struct IconXPMCache;

namespace yasli { class Object; struct Context; }


namespace property_tree{ 
struct IUIFacade; 
struct KeyEvent;
}
using property_tree::IUIFacade;
using property_tree::KeyEvent;

class PropertyTree : public TreeConfig, public sigslot::has_slots
{
public:
	void attach(const yasli::Object& object);
	void attach(const yasli::Serializer& serializer);
	bool attach(const yasli::Serializers& serializers);
	virtual void attachPropertyTree(PropertyTree* propertyTree);
	void getSelectionSerializers(yasli::Serializers* serializers);
	void detach();
	bool attached() const { return !attached_.empty(); }

	void apply(bool continuous = false);
	void revert();
	void revertNonInterrupting();
	int revertObjects(vector<void*> objectAddresses);
	bool revertObject(void* objectAddress);

	void setCompact(bool compact) { compact_ = compact; repaint(); }
	void setFullRowMode(bool fullRowMode) { fullRowMode_ = fullRowMode; repaint(); }
	void setExpandLevels(int levels);
	void setUndoEnabled(bool enabled, bool full = false);
	void setAutoRevert(bool autoRevert) { autoRevert_ = autoRevert; }
	void setArchiveContext(yasli::Context* lastContext);
	bool multiSelectable() const { return attachedPropertyTree_ != 0; }

	Point treeSize() const;

	int leftBorder() const { return leftBorder_; }
	int rightBorder() const { return rightBorder_; }

	bool spawnWidget(PropertyRow* row, bool ignoreReadOnly);
	PropertyRow* selectedRow();
	bool getSelectedObject(yasli::Object* object);
	bool selectByAddress(const void*, bool keepSelectionIfChildSelected = false);
	bool selectByAddresses(const vector<const void*>& addresses, bool keepSelectionIfChildSelected);
	void ensureVisible(PropertyRow* row, bool update = true);
	void expandParents(PropertyRow* row);
	void expandRow(PropertyRow* row, bool expanded = true, bool updateHeights = true);
	void expandAll();
	void collapseAll();
	void expandChildren(PropertyRow*);
	void collapseChildren(PropertyRow*);

	virtual void onSignalChanged() = 0;
	virtual void serialize(yasli::Archive& ar) = 0;

protected:
	PropertyTree(IUIFacade* ui);
	~PropertyTree();

public:
	// internal use

	PropertyTreeModel* model() { return model_.get(); }
	const PropertyTreeModel* model() const { return model_.get(); }
	IUIFacade* ui() const { return ui_; }
	void _cancelWidget(){ defocusInplaceEditor(); widget_.reset(); }
	virtual bool _isDragged(const PropertyRow* row) const = 0;
	bool _isCapturedRow(const PropertyRow* row) const;

	PropertyRow* _pressedRow() const { return pressedRow_; }
	void _setPressedRow(PropertyRow* row) { pressedRow_ = row; }
	int _applyTime() const{ return applyTime_; }
	int _revertTime() const{ return revertTime_; }
	virtual bool hasFocusOrInplaceHasFocus() const = 0;
	void addMenuHandler(PropertyRowMenuHandler* handler);
	void clearMenuHandlers();
	Point _toWidget(Point point) const;
	virtual void repaint() = 0;
	virtual void updateHeights() = 0;
	virtual void defocusInplaceEditor() = 0;

	virtual void signalAboutToSerialize(yasli::Archive& ar) = 0;
	virtual void signalChanged() = 0;
	virtual void signalContinuousChange() = 0;
	virtual void signalSelected() = 0;
	virtual void signalReverted() = 0;
	virtual void signalPushUndo() = 0;

protected:
	virtual bool canBePasted(PropertyRow* destination) = 0;
	virtual bool canBePasted(const char* destinationType) = 0;
	PropertyRow* rowByPoint(const Point& point);
	void onRowMenuDecompose(PropertyRow* row);
	bool toggleRow(PropertyRow* row);

	struct RowFilter {
		enum Type {
			NAME_VALUE,
			NAME,
			VALUE,
			TYPE,
			NUM_TYPES
		};

		yasli::string start[NUM_TYPES];
		bool tillEnd[NUM_TYPES];
		std::vector<yasli::string> substrings[NUM_TYPES];

		void parse(const char* filter);
		bool match(const char* text, Type type, size_t* matchStart, size_t* matchEnd) const;
		bool typeRelevant(Type type) const{
			return !start[type].empty() || !substrings[type].empty();
		}

		RowFilter()
		{
			for (int i = 0; i < NUM_TYPES; ++i)
				tillEnd[i] = false;
		}
	};

	Point pointToRootSpace(const Point& pointInWindowSpace) const;
	virtual bool updateScrollBar() = 0;
	virtual void interruptDrag() = 0;
	virtual void _arrangeChildren() = 0;
	virtual void resetFilter() = 0;

	bool onContextMenu(PropertyRow* row, IMenu& menu);

	void onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos);
	bool onRowKeyDown(PropertyRow* row, const KeyEvent* ev);
	// points here are specified in root-row space
	bool onRowLMBDown(PropertyRow* row, const Rect& rowRect, Point point, bool controlPressed);
	void onRowLMBUp(PropertyRow* row, const Rect& rowRect, Point point);
	void onRowRMBDown(PropertyRow* row, const Rect& rowRect, Point point);
	void onRowMouseMove(PropertyRow* row, const Rect& rowRect, Point point);
	void onMouseStill();

	bool activateRow(PropertyRow* row);

	void setWidget(InplaceWidget* widget, PropertyRow* widgetRow);

	void updateAttachedPropertyTree(bool revert);

	void onModelUpdated(const PropertyRows& rows, bool needApply);
	virtual void onModelPushUndo(PropertyTreeOperator* op, bool* handled);

private:
	PropertyTree(const PropertyTree&);
	PropertyTree& operator=(const PropertyTree&);
protected:
	std::auto_ptr<PropertyTreeModel> model_;
	std::auto_ptr<InplaceWidget> widget_; // in-place widget
	PropertyRow* widgetRow_;
	vector<PropertyRowMenuHandler*> menuHandlers_;
	IUIFacade* ui_;

	typedef vector<yasli::Object> Objects;
	Objects attached_;
	PropertyTree* attachedPropertyTree_;
	RowFilter rowFilter_;
	std::auto_ptr<IconXPMCache> iconCache_; 
	yasli::Context* archiveContext_;

	int leftBorder_;
	int rightBorder_;
	Point size_;
	Point offset_;
	Rect area_;

	int cursorX_;
	bool filterMode_;
	Point pressPoint_;
	Point lastStillPosition_;
	PropertyRow* capturedRow_;
	PropertyRow* pressedRow_;

	bool autoRevert_;
	int applyTime_;
	int revertTime_;

	bool dragCheckMode_;
	bool dragCheckValue_;

	friend class DragWindow;
	friend class QDrawContext;
	friend struct FilterVisitor;
	friend struct PropertyTreeMenuHandler;
    friend struct ContainerMenuHandler;
	friend class PropertyTreeModel;
};
