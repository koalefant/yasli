#pragma once

#include <wx/gdicmn.h> // wxRect
#include <wx/bitmap.h>
#include "yasli/TypeID.h"
#include "utils/sigslot.h"

class wxDC;
class wxWindow;

inline wxString wxStringFromUTF8(const char* utf8_str)
{
#if WX_USE_UNICODE
	return wxString(utf8_str, wxConvUTF8);
#else
	return wxString(wxConvUTF8.cMB2WC(utf8_str), *wxConvCurrent);
#endif
}

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

#define PROPERTY_ITEM_CLONE_IMPL(Type) \
	Type* clone() const{ \
		ASSERT(sizeof(Type) == sizeof(*this)); \
        return new Type(*this); \
	}

class PopupMenu;

class PropertyItem : public RefCounter, public PropertyItemState{
public:
    enum{
        SERIALIZE_STATE = 1,
        SERIALIZE_CONTENT = 2
    };

	enum ContextMenuSection{
		MENU_SECTION_MAIN,
		MENU_SECTION_CLIPBOARD,
		MENU_SECTION_DESTRUCTIVE
	};

    PropertyItem(const char* name = "", TypeID type = TypeID());
    PropertyItem(const PropertyItem& original);
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

	virtual PropertyItem* clone() const = 0;
    virtual bool isLeaf() const{ return true; }
    virtual bool isContainer() const{ return false; }
    virtual bool isElement() const{ return false; }

    virtual bool hit(wxCoord coord, const ViewContext& context) const{ return false; }
    virtual bool hitExpandArea(wxCoord coord, const ViewContext& context) const{ return false; }
    virtual bool hitSelectArea(wxCoord coord, const ViewContext& context) const{ return false; }

    virtual void redraw(wxDC& dc, const ViewContext& context) const;
	virtual bool activate(const ViewContext& context);
    virtual bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context);
    virtual bool onRightMouseClick(wxPoint pos, const ViewContext& context);
    virtual void onContextMenu(PopupMenu& menu, PropertyTree* tree, ContextMenuSection section);

	void setLabel(const char* label){ label_ = label; }
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
	PropertyItem* findByIndex(int index);
	const PropertyItem* findByIndex(int index) const;
	int findIndexOf(const PropertyItem* child) const;
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
    PropertyItemCheck(const PropertyItemCheck& original);

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

class PropertyWithButtons{
public:
    typedef PropertyItem::ViewContext ViewContext;

    PropertyWithButtons()
    {
    }

    PropertyWithButtons(const PropertyWithButtons& original)
    : buttons_(original.buttons_)
    {
    }

	bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context, const PropertyItemRects& rects);
	wxRect calculateButtonsRect(const ViewContext& context, const PropertyItemRects& rects) const;
    void redraw(wxDC& dc, const ViewContext& context, const PropertyItemRects& rects) const;

	void addButton(const wxBitmap* bitmap){
		Button button;
		button.bitmap = bitmap;
		buttons_.push_back(button);
	}
	int buttonsCount() const{ return buttons_.size(); }
protected:
	virtual bool onButton(int index, const ViewContext& context) = 0;
private:
	struct Button{
		const wxBitmap* bitmap;
	};
	std::vector<Button> buttons_;
};

class PropertyItemField : public PropertyItem, public PropertyWithControl, public PropertyWithButtons{
public:
	using PropertyItem::ViewContext;
    PropertyItemField(const char* name = "", TypeID type = TypeID());
    PropertyItemField(const PropertyItemField& original);
    void calculateRects(const PropertyItem::ViewContext& context, PropertyItemRects& rects) const;
    wxRect calculateFieldRect(const ViewContext& context, const PropertyItemRects& rects) const;
	bool activate(const ViewContext& context);
    void redraw(wxDC& dc, const ViewContext& context) const;
    bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context);
	virtual std::string toString() const;
    virtual void fromString(const char* str) {}

    PropertyControl* createControl(const ViewContext& context);
protected:
    // from PropertyWithButtons
	bool onButton(int index, const ViewContext& context){
		return activate(context);
	}
    // ^^^
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
class ContainerSerializationInterface;
class PropertyItemContainer : public PropertyItem, public PropertyWithButtons, public sigslot::has_slots<>{
public:
	using PropertyItem::ViewContext;
    PropertyItemContainer();
    PropertyItemContainer(const PropertyItemContainer& original);
    PropertyItemContainer(const char* name, const ContainerSerializationInterface& ser);

    // from PropertyItem:
    PropertyItemContainer* clone() const{
        return new PropertyItemContainer(*this);
    }
    bool isContainer() const{ return true; }
	bool activate(const ViewContext& context);
    void redraw(wxDC& dc, const ViewContext& context) const;
    bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context);
    void onContextMenu(PopupMenu& menu, PropertyTree* tree, ContextMenuSection section);
    // ^^^
	void onElementContextMenu(PropertyItem* element, PopupMenu& menu, PropertyTree* tree, ContextMenuSection section);
protected:
    void onMenuAddElement(PropertyTree* tree);
    void onMenuRemoveAllElements(PropertyTree* tree);
    void onMenuRemoveElement(PropertyTree* tree, PropertyItem* element);

    // from PropertyWithButtons:
	bool onButton(int index, const ViewContext& context);
    // ^^^

    TypeID childrenType_;
    bool fixedSize_;
};

class PropertyItemElement : public PropertyItem{
public:
    PropertyItemElement(const char* name = "", TypeID type = TypeID())
    : PropertyItem(name, type)
    {
    }
	PropertyItemElement(const PropertyItemElement& original)
	: PropertyItem(original)
	{
	}

    bool isElement() const{ return true; }
	PROPERTY_ITEM_CLONE_IMPL(PropertyItemElement)
protected:
};


// ---------------------------------------------------------------------------

class PropertyItemElement;
class PropertyTreeRoot : public PropertyItem{
public:
    PropertyTreeRoot()
    {
    }
    bool isRoot() const{ return true; }
    bool isLeaf() const{ return false; }

    bool hasElement(TypeID typeID) const;
    void addElement(TypeID typeID, PropertyItem* element);
    const PropertyItem* findElement(TypeID type) const;

	//typedef std::vector<SharedPtr<PropertyItem> > DerivedTypes;
    //typedef std::map<TypeID, DerivedTypes> DerivedTypesByBase;

    typedef std::map<TypeID, SharedPtr<PropertyItem> > ElementsByType;

	PropertyTreeRoot* clone() const{ return 0; }
protected:
    ElementsByType elementsByType_;

};

