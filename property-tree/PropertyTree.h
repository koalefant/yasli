#pragma once
#include <wx/scrolwin.h>
#include "yasli/sigslot.h"
#include "yasli/FrozenSignal.h"
#include "yasli/Serializer.h"
#include "PropertyItem.h"

class wxDC;

class PropertyWithControl;
class LibrarySelector;

class PropertyItemPath
{
public:
	PropertyItem* get(PropertyItem& root) const;
	void set(PropertyItem& root, PropertyItem* item);

private:
	typedef std::vector<int> Indices;
	Indices indices_;
};

class PropertyTree : public wxScrolledWindow,
      public sigslot::has_slots<>, public Refrigerator{
public:
    PropertyTree(wxWindow* parent = 0, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxSize(0, 0),
                 const wxValidator &validator = wxDefaultValidator,
                 const wxString& name = wxT("PropertyTree"));
    ~PropertyTree();

    void serialize(Archive& ar);

    void attach(Serializer s);
    void detach();

    void revert();
	void apply();

    void select(PropertyItem* item, bool exclusive = true);
    void toggle(PropertyItem* item);

    void deselectAll(PropertyItem* root = 0);
	void expandAll(int onlyFirstLevels = 0);

    PropertyTreeRoot* root(){ return &root_; }

    typedef FrozenSignal<> SignalChanged;
    SignalChanged& signalChanged(){ return signalChanged_; }

    PropertyItem* focusedItem();
    void setFocusedItem(PropertyItem* focusedItem);

	void spawnContextMenu(const PropertyItem::ViewContext& context, PropertyItem* item);
    void spawnInPlaceControl(const PropertyItem::ViewContext& context, PropertyWithControl* item);
    void cancelControl(bool now = true, bool commit = true);
	
	void commitChange(PropertyItem* changedOne);
	void referenceFollowed(LibrarySelector& selector);
	typedef sigslot::signal1<LibrarySelector&> SignalReferenceFollowed;
	SignalReferenceFollowed& signalReferenceFollowed(){ return signalReferenceFollowed_; }
protected:
    void redraw(wxDC& dc);

    void onPaint(wxPaintEvent &event);
    void onMouseClick(wxMouseEvent& event);
    void onRightMouseClick(wxMouseEvent& Event);
    void onSize(wxSizeEvent& event);
    void onSetFocus(wxFocusEvent& event);
    void onKillFocus(wxFocusEvent& event);
    void onKeyDown(wxKeyEvent& event);
    void onCancelControl(wxCommandEvent& event);

	void onMenuCopy(PropertyItem* item);
	void onMenuPaste(PropertyItem* item);
	bool canPasteOver(PropertyItem* item);

    void initViewContext(PropertyItem::ViewContext& context, PropertyItem* forItem = 0);
	PropertyFilter& currentFilter();

    SignalChanged signalChanged_;
	SignalReferenceFollowed signalReferenceFollowed_;
    void onChanged(Refrigerator* changer);

    Serializer attached_;

    SharedPtr<PropertyControl> spawnedControl_;

    PropertyItemPath focusedItemPath_;
    PropertyTreeRoot root_;
	bool redrawing_;

    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(PropertyTree)
};

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_PROPERTY_TREE_CANCEL_CONTROL, -1)
END_DECLARE_EVENT_TYPES()
