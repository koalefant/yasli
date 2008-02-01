#pragma once
#include <wx/gdicmn.h> // wxRect
#include <wx/bitmap.h>
#include "yasli/TypeID.h"
class wxDC;
class wxWindow;

struct PropertyItemRects;
class PropertyItem;
struct PropertyFilter{
    virtual ~PropertyFilter() {}
    virtual bool operator()(const PropertyItem& item){ return true; }
};

class PropertyTree;

enum SiblingDirection{
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT
};

struct PropertyItemRects{
    wxRect text;
    wxRect indent;
    wxRect selection;
    wxRect plus;
};

class Archive;
class PropertyItemState{
public:
    PropertyItemState();
    virtual ~PropertyItemState() {}

    virtual void setExpanded(bool expanded){ expanded_ = expanded; }
    bool expanded() const{ return expanded_; }

    void setSelected(bool selected){ selected_ = selected; }
    bool selected() const{ return selected_; }

    void serialize(Archive& ar);
protected:
    bool selected_;
    bool expanded_;
};

#define PROPERTY_ITEM_ASSIGN_IMPL(Type) \
	void assignTo(void* data, int size) const{ \
		ASSERT(sizeof(Type) == size); \
		*reinterpret_cast<Type*>(data) = value_; \
	}

class PropertyItem : public RefCounter, public PropertyItemState{
public:
    enum{
        SERIALIZE_STATE = 1,
        SERIALIZE_CONTENT = 2
    };

    PropertyItem(const char* name = "", TypeID type = TypeID());
    virtual ~PropertyItem() {}

	typedef std::vector<SharedPtr<PropertyItem> > Children;

    PropertyItem* add(PropertyItem* newChild);
    PropertyItem* addAfter(PropertyItem* newChild, PropertyItem* after);
    PropertyItem* replace(PropertyItem* item, PropertyItem* withItem);
    void remove(PropertyItem* item);
    void clear();
    void serialize(Archive& ar);

    struct ViewContext{
        ViewContext()
        : tree(0)
        , focusedItem(0)
        {}
        PropertyTree* tree;
        PropertyItem* focusedItem;
        wxRect visibleRect;
        wxRect rect;
    };

    virtual bool isLeaf() const{ return true; }
    virtual bool isContainer() const{ return false; }

    virtual bool hit(wxCoord coord, const ViewContext& context) const{ return false; }
    virtual bool hitExpandArea(wxCoord coord, const ViewContext& context) const{ return false; }
    virtual bool hitSelectArea(wxCoord coord, const ViewContext& context) const{ return false; }

    virtual void redraw(wxDC& dc, const ViewContext& context) const;
	virtual bool activate(const ViewContext& context);
    virtual bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context);

    const char* label() const{ return label_.c_str(); }
    const char* name() const{ return name_.c_str(); }


    virtual bool hasVisibleChildren() const{ return !children_.empty(); }

    void setParent(PropertyItem* newParent);
    PropertyItem* parent(){ return parent_; }
    const PropertyItem* parent() const{ return parent_; }

	void prepareContext(ViewContext& context, const PropertyItem* parent, const ViewContext& parentContext) const;
    wxRect calculateChildRect(const PropertyItem* child, const wxRect& rect) const;
    virtual void calculateRects(const PropertyItem::ViewContext& context, PropertyItemRects& rects) const;
	virtual int calculateHeight(PropertyFilter& visibleFilter, int offset);
    int height() const{ return height_; }
    void wipe(){ height_ = 0; }

    virtual bool isRoot() const{ return false; }

    bool layoutChanged() const{ return layoutChanged_; }
	bool updated() const{ return updated_; }
	void setChildrenUpdated(bool updated);

    virtual PropertyItem* findSibling(PropertyItem* forItem, SiblingDirection dir, bool enterExpanded = true);
	PropertyItem* findByName(const char* name, bool noUpdated);
    virtual bool isChildVisible(PropertyItem* child, PropertyFilter& filter) const;

    typedef Children::iterator iterator;
    iterator begin(){ return children_.begin(); }
    iterator end(){ return children_.end(); }
    bool empty() const{ return children_.empty(); }
	size_t size() const{ return children_.size(); }
	iterator erase(iterator it){ return children_.erase(it); }

    typedef Children::const_iterator const_iterator;
    const_iterator begin() const{ return children_.begin(); }
    const_iterator end() const{ return children_.end(); }

	virtual void assignTo(void* data, int size) const{ ASSERT(0); }
protected:
	void assignStateFrom(PropertyItem* item);
	void swapChildrenWith(PropertyItem* item);

	int offset_;
    int height_;
    PropertyItem* parent_;
    bool layoutChanged_;
	bool updated_;
	std::string name_;
    std::string label_;
    TypeID type_;
    Children children_;
};

class PropertyControl;

class PropertyWithControl{
public:
    virtual ~PropertyWithControl() {}
    virtual PropertyControl* createControl(const PropertyItem::ViewContext& context) { return 0; }
    virtual wxRect controlPosition(const PropertyItem::ViewContext& context){
        return wxRect();
    }
};

class PropertyItemCheck : public PropertyItem{
public:
    PropertyItemCheck(const char* name = "", TypeID type = TypeID());

    void calculateRects(const PropertyItem::ViewContext& context, PropertyItemRects& rects) const;
    wxRect calculateCheckRect(const ViewContext& context, const PropertyItemRects& rects) const;

	bool activate(const ViewContext& context);
	void redraw(wxDC& dc, const ViewContext& context) const;
	bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context);

	int addCheckState(wxBitmap& bitmap);
	void addDefaultCheckStates();
	virtual int stateIndex() const = 0;
protected:
	std::vector<wxBitmap> checkStates_;
};

class PropertyItemField : public PropertyItem, public PropertyWithControl{
public:
    PropertyItemField(const char* name = "", TypeID type = TypeID());
    void calculateRects(const PropertyItem::ViewContext& context, PropertyItemRects& rects) const;
    wxRect calculateFieldRect(const ViewContext& context, const PropertyItemRects& rects) const;
	wxRect calculateButtonsRect(const ViewContext& context, const PropertyItemRects& rects) const;
	int buttonsCount() const{ return buttons_.size(); }
	bool activate(const ViewContext& context);
    void redraw(wxDC& dc, const ViewContext& context) const;
    bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context);
	virtual std::string toString() const;
    virtual void fromString(const char* str) {}

    PropertyControl* createControl(const ViewContext& context);
protected:
	void addButton(const wxBitmap* bitmap){
		Button button;
		button.bitmap = bitmap;
		buttons_.push_back(button);
	}
	virtual bool onButton(int index, const ViewContext& context){
		return activate(context);
	}
private:
	struct Button{
		const wxBitmap* bitmap;
	};
	std::vector<Button> buttons_;
};


class PropertyControl : public RefCounter{
public:
    PropertyControl(PropertyWithControl* property, PropertyTree* tree)
    : property_(property)
    , tree_(tree)
    {
		ASSERT(tree != 0);
	}
    virtual ~PropertyControl(){}

    virtual void cancel() {}
    virtual void commit() {}
    virtual wxWindow* get() = 0;

    PropertyWithControl* property(){ return property_; }
    PropertyTree* tree(){ return tree_; }
	wxRect scrollRect(const wxRect& rect);
protected:
    PropertyWithControl* property_;
    PropertyTree* tree_;
};


// ---------------------------------------------------------------------------

class PropertyTreeRoot : public PropertyItem{
public:
    PropertyTreeRoot()
    {
    }
    bool isRoot() const{ return true; }
    bool isLeaf() const{ return false; }
protected:

};

