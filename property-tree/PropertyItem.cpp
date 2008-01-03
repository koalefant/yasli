#include "StdAfx.h"
#include <algorithm>

#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"
#include "yasli/PointersImpl.h"

#include "yasli/TypesFactory.h"
#include <wx/font.h>
#include <wx/settings.h>
#include <wx/dc.h>
#include <wx/textctrl.h>

#include "PropertyItem.h"
#include "PropertyItemBasic.h"
#include "PropertyItemContainer.h"

#include "PropertyTree.h"

PropertyItemState::PropertyItemState()
: selected_(false)
, expanded_(false)
{
}

void PropertyItemState::serialize(Archive& ar)
{
    ar(selected_, "s");
    ar(expanded_, "e");
}

// ---------------------------------------------------------------------------

static const int DEFAULT_HEIGHT = 21;
static const int DEFAULT_PADDING = 1;
static const int DEFAULT_INDENT = 8;

static wxFont rowFontDefault;
static wxFont rowFontBold;

PropertyItem::PropertyItem(const char* name, TypeID type)
: height_(0)
, parent_(0)
, layoutChanged_(true)
, updated_(true)
, name_(name)
, label_(name)
, type_(type)
{
    static bool fontsInitialized = false;
    if(!fontsInitialized){
        rowFontDefault = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
        rowFontBold = rowFontDefault;
        rowFontBold.SetWeight(wxFONTWEIGHT_BOLD);
        fontsInitialized = true;
    }
}

void PropertyItem::assignStateFrom(PropertyItem* item)
{
	expanded_ = item->expanded_;
	selected_ = item->selected_;
}

void PropertyItem::swapChildrenWith(PropertyItem* item)
{
	Children temp = children_;
	children_ = item->children_;
	item->children_ = temp;

	iterator it;
	FOR_EACH(children_, it){
		PropertyItem* item = *it;
		item->setParent(this);
	}
	FOR_EACH(*item, it){
		PropertyItem* item = *it;
		item->setParent(item);
	}
}


int PropertyItem::calculateHeight(PropertyFilter& visibleFilter, int offset)
{
    height_ = isRoot() ? 0 : DEFAULT_HEIGHT;
    height_ += DEFAULT_PADDING;
	offset_ = offset;

	int childOffset = offset + height_;
    Children::iterator it;
    FOR_EACH(children_, it){
        PropertyItem* child = *it;
        ASSERT(child);

		int height = 0;
        if(isChildVisible(child, visibleFilter))
            height = child->calculateHeight(visibleFilter, childOffset);
        else
            child->wipe();
		height_ += height;
		childOffset += height;
    }

    height_ += DEFAULT_PADDING;
//	layoutChanged_ = false;
    return height_;
}

bool PropertyItem::isChildVisible(PropertyItem* child, PropertyFilter& filter) const
{
    return expanded_ && filter(*child);
}

void PropertyItem::setParent(PropertyItem* newParent)
{
    parent_ = newParent;
}

PropertyItem* PropertyItem::add(PropertyItem* newChild)
{
    children_.push_back(newChild);
    newChild->setParent(this);
    return newChild;
}

PropertyItem* PropertyItem::addAfter(PropertyItem* newChild, PropertyItem* after)
{
	if(after == 0){
		children_.insert(children_.begin(), newChild);
		newChild->setParent(this);
		return newChild;
	}
	iterator it = std::find(children_.begin(), children_.end(), after);
	ASSERT(it != children_.end());
	++it;
	children_.insert(it, newChild);
	newChild->setParent(this);
	return newChild;
}

PropertyItem* PropertyItem::replace(PropertyItem* item, PropertyItem* withItem)
{
	iterator it = std::find(children_.begin(), children_.end(), item);
	ASSERT(it != children_.end());
	SharedPtr<PropertyItem> ref = item;
	item->setParent(0);
	*it = withItem;
	withItem->setParent(this);
	withItem->swapChildrenWith(item);
	withItem->assignStateFrom(item);    
    return withItem;
}

void PropertyItem::remove(PropertyItem* item)
{
	iterator it = std::find(children_.begin(), children_.end(), item);
	ASSERT(it != children_.end());
	item->setParent(0);
	children_.erase(it);
}

void PropertyItem::clear()
{
    children_.clear();
}

void PropertyItem::serialize(Archive& ar)
{
    if(ar.filter(SERIALIZE_STATE)){
        PropertyItemState::serialize(ar);

        int count = children_.size();
        ar(count, "");
        count = std::min(int(children_.size()), count);
        for(int i = 0; i < count; ++i){
            ar(*children_[i], "");
        }
    }
    if(ar.filter(SERIALIZE_CONTENT)){
        ar(children_, "children");
    }
}


static const int PLUS_INDENT = 16;
#include "res/property_tree_plus.xpm"
#include "res/property_tree_minus.xpm"



void PropertyItem::calculateRects(const PropertyItem::ViewContext& context, PropertyItemRects& rects) const
{
    wxRect rect = context.rect;
    if(hasVisibleChildren() && !isRoot()){
        rects.selection = wxRect(rect.x + PLUS_INDENT + 1, rect.y + 1,
                                 rect.width - (PLUS_INDENT) - 2, DEFAULT_HEIGHT - 2);
    }
    else
        rects.selection = wxRect(rect.x + PLUS_INDENT + 1, rect.y + 1,
                                 rect.width - (PLUS_INDENT) - 2, DEFAULT_HEIGHT - 2);


    rects.text = context.rect;
    rects.text.x += 3 + PLUS_INDENT;
	rects.text.width -= 6 + PLUS_INDENT;
    rects.text.y += 2;
    rects.text.height = DEFAULT_HEIGHT - 4;

    rects.plus = context.rect;
    rects.plus.width = 16;
    rects.plus.height = 16;
    rects.plus.y += (rects.text.height - 16) / 2 + 2;

    if(label()[0] == '\0')
        rects.text.width = 0;
    else
        rects.text.width = context.visibleRect.width - rects.text.x;

    rects.indent = wxRect(rect.x, rect.y, DEFAULT_INDENT, rect.height);
}

wxRect PropertyItem::calculateChildRect(const PropertyItem* child, const wxRect& rect) const
{
    ASSERT(child);
    if(isRoot()){
        return wxRect(rect.x + DEFAULT_PADDING,
                      child->offset_ + DEFAULT_PADDING,
                      rect.width - DEFAULT_PADDING * 2,
                      child->height());
    }
    else{
		return wxRect(rect.x + DEFAULT_PADDING + DEFAULT_INDENT,
					  child->offset_ + DEFAULT_PADDING,
					  rect.width - (DEFAULT_PADDING + DEFAULT_INDENT),
					  child->height());
    }
}

void PropertyItem::prepareContext(ViewContext& context, const PropertyItem* parent, const ViewContext& parentContext) const
{
	ASSERT(parent);
	context = parentContext;
    context.rect = parent->calculateChildRect(this, parentContext.rect);
}

bool PropertyItem::onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context)
{
    PropertyItemRects rects;
    calculateRects(context, rects);
    
    if(!isRoot()){
        bool inPlusRect = rects.plus.Contains(pos);
        bool inSelectionRect = rects.selection.Contains(pos);
        bool inTextRect = rects.text.Contains(pos);
        if(!doubleClick && inSelectionRect){
            std::cout << "Selecting..." << std::endl;
            context.tree->select(this, true);
            return true;
        }
        else if((!doubleClick && inPlusRect) || (doubleClick && inTextRect)){
            std::cout << "Toggling..." << std::endl;
            context.tree->toggle(this);
            return true;
        }
    }

    if(!expanded())
        return false;

    iterator it;
    FOR_EACH(*this, it){
        PropertyItem* child = *it;

		ViewContext childContext;
		child->prepareContext(childContext, this, context);
        if(child->onMouseClick(pos, doubleClick, childContext))
			return true;
    }
	return false;
}

bool PropertyItem::activate(const ViewContext& context)
{
	return false;
}

void PropertyItem::redraw(wxDC& dc, const ViewContext& context) const
{
#ifdef WIN32
    static wxBitmap plusBitmap((const char* const*)property_tree_plus_xpm);
    static wxBitmap minusBitmap((const char* const*)property_tree_minus_xpm);
#else
    static wxBitmap plusBitmap(property_tree_plus_xpm);
    static wxBitmap minusBitmap(property_tree_minus_xpm);
#endif

    PropertyItemRects rects;
    calculateRects(context, rects);

	/* DEBUG
	dc.SetPen(*wxRED_PEN);
	dc.DrawRectangle(context.rect);
	*/

    if(hasVisibleChildren()){
        dc.SetFont(rowFontBold);
        //dc.SetFont(rowFontDefault);
        if(!isRoot())
            dc.DrawBitmap(expanded() ? minusBitmap : plusBitmap, rects.plus.x, rects.plus.y, true);
    }
    else{
        dc.SetFont(rowFontDefault);
    }
    wxBrush brush;
    wxColour textColor;
    if(selected()){
        brush = wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    }
    else{
        brush = *wxTRANSPARENT_BRUSH;
        textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    }

    dc.SetTextForeground(textColor);
    if(selected()){
        dc.SetBrush(brush);

		dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(rects.selection);

        if(context.focusedItem == this){
			wxPen focusPen(wxColour(0, 0, 0), 1, wxSOLID);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(focusPen);
            dc.SetLogicalFunction(wxINVERT);
            wxRect focusRect = rects.selection;
            focusRect.Inflate(1, 1);
            dc.DrawRectangle(focusRect/*, radius + 1*/);
            dc.SetLogicalFunction(wxCOPY);
        }
    }
    dc.DrawLabel(wxString(label(), wxConvUTF8), wxBitmap(),
                 rects.text, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    if(expanded()){
        Children::const_iterator it;
        wxRect previousChildRect(0, 0, 0, 0);
        FOR_EACH(children_, it){
            const PropertyItem* child = *it;
            ViewContext childContext;
			child->prepareContext(childContext, this, context);
            child->redraw(dc, childContext);
            previousChildRect = childContext.rect;
        }
    }
}

void PropertyItem::setChildrenUpdated(bool updated)
{
	iterator it;
	FOR_EACH(*this, it){
		PropertyItem* child = *it;
		child->updated_ = updated;
	}
}

PropertyItem* PropertyItem::findSibling(PropertyItem* forItem, SiblingDirection dir, bool enterExpanded)
{
    switch(dir){
    case DIR_UP:
        {
            if(forItem == 0){
                if(expanded() && !empty()){
                    PropertyItem* last = children_.back();
                    if(last->expanded())
                        return last->findSibling(0, DIR_UP);
                    else
                        return last;
                }
                else
                    return this;
            }
            iterator it = std::find(begin(), end(), forItem);
            ASSERT(it != end());
            if(it == end())
                return 0;
            if(it == begin())
                return this;
            --it;
            PropertyItem* item = *it;
            if(item->expanded() && !item->empty())
                return item->findSibling(0, DIR_UP);
            else
                return item;
        }
    case DIR_DOWN:
        {
            if(forItem->expanded() && enterExpanded){
                if(!forItem->empty()){
                    if(forItem == this)
                        return *begin();
                    PropertyItem* item = forItem->findSibling(forItem, DIR_DOWN);
                    if(item)
                        return item;
                }
            }
            iterator it = std::find(begin(), end(), forItem);
            ASSERT(it != end());
            if(it == end())
                return 0;
            ++it;
            if(it == end()){
                if(parent())
                    return parent()->findSibling(this, DIR_DOWN, false);
                else
                    return 0;
            }
            return *it;
        }
    default:
        return 0;
    }
}

PropertyItem* PropertyItem::findByName(const char* name, bool noUpdated)
{
	iterator it;
	FOR_EACH(*this, it){
		PropertyItem* child = *it;
		if((!noUpdated || !child->updated()) && strcmp(child->name(), name) == 0)
			return child;
	}
	return 0;
}

// ---------------------------------------------------------------------------

wxRect PropertyControl::scrollRect(const wxRect& rect)
{
	wxPoint position = tree()->CalcScrolledPosition(rect.GetPosition());
	return wxRect(position, rect.GetSize());
}

// ---------------------------------------------------------------------------

class PropertyControlText : public wxTextCtrl, public PropertyControl{
public:
    PropertyControlText(const PropertyItem::ViewContext& context,
                        PropertyItemField* field)
    : wxTextCtrl()
    , PropertyControl(field, context.tree)
    {
        PropertyItemRects rects;
        field->calculateRects(context, rects);
        wxRect rect = field->calculateFieldRect(context, rects);
		rect.SetPosition(context.tree->CalcScrolledPosition(rect.GetPosition()));
		wxTextCtrl::Create(context.tree, wxID_ANY, wxT(""), rect.GetPosition(), rect.GetSize(), wxTE_PROCESS_ENTER);
        SetValue(wxString(field->toString().c_str(), wxConvUTF8));
        SetFocus();
		SelectAll();
    }
    ~PropertyControlText(){
    }

    void commit(){
        PropertyItemField* field = safe_cast<PropertyItemField*>(property());
        field->fromString(GetValue().utf8_str());
		tree()->commitChange(field);
    }

    void onKillFocus(wxFocusEvent& event){
		ASSERT(tree());
		//if(tree())
		tree()->cancelControl();
    }

    wxWindow* get(){ return this; }

    DECLARE_CLASS(PropertyControlText)
    DECLARE_EVENT_TABLE()
protected:
};

IMPLEMENT_CLASS(PropertyControlText, wxTextCtrl)
BEGIN_EVENT_TABLE(PropertyControlText, wxTextCtrl)
    EVT_KILL_FOCUS(PropertyControlText::onKillFocus)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------


PropertyItemField::PropertyItemField(const char* name, TypeID type)
: PropertyItem(name, type)
{

}

void PropertyItemField::calculateRects(const PropertyItem::ViewContext& context, PropertyItemRects& rects) const
{
    PropertyItem::calculateRects(context, rects);
    if(label()[0] != '\0'){
        rects.text.width = context.visibleRect.width / 2 - rects.text.x;
    }
}

static int BUTTON_PADDING = 1;
static int BUTTON_BORDER = 1;
static int BUTTON_SIZE = 16;
static int BUTTON_SPACING = BUTTON_SIZE + BUTTON_BORDER * 2 + BUTTON_PADDING;

wxRect PropertyItemField::calculateFieldRect(const ViewContext& context, const PropertyItemRects& rects) const
{

	int left;
	if((label()[0] != '\0'))
		left = rects.text.GetRight();
	else
		left = PLUS_INDENT + context.rect.x + 1;
    return wxRect(left,
				  context.rect.y + 1,
                  context.rect.GetRight() - left - BUTTON_SPACING * buttons_.size(),
                  DEFAULT_HEIGHT - 2);

}

wxRect PropertyItemField::calculateButtonsRect(const ViewContext& context, const PropertyItemRects& rects) const
{
	int left = context.rect.GetRight() - buttons_.size() * BUTTON_SPACING;
	return wxRect(left, context.rect.y + 1, context.rect.GetRight() - left, DEFAULT_HEIGHT - 2);
}

PropertyControl* PropertyItemField::createControl(const ViewContext& context)
{
    return new PropertyControlText(context, this);
}

std::string PropertyItemField::toString() const
{
    return label_;
}

bool PropertyItemField::activate(const ViewContext& context)
{
    context.tree->spawnInPlaceControl(context, this);
	return false;
}

bool PropertyItemField::onMouseClick(wxPoint pos, bool doubleClick,
                                     const ViewContext& context)
{
    PropertyItemRects rects;
    calculateRects(context, rects);
    wxRect fieldRect = calculateFieldRect(context, rects);
	wxRect buttonsRect = calculateButtonsRect(context, rects);

    if(rects.text.Contains(pos) && doubleClick)
		if(activate(context))
			return true;
    if(fieldRect.Contains(pos))
		if(activate(context))
			return true;
	if(!buttons_.empty() && buttonsRect.Contains(pos)){
		int index = (pos.x - buttonsRect.GetLeft()) / int(BUTTON_SPACING);
		if(onButton(index, context))
			return true;
	}
    return PropertyItem::onMouseClick(pos, doubleClick, context);
}

void PropertyItemField::redraw(wxDC& dc, const ViewContext& context) const
{
    PropertyItemRects rects;
    calculateRects(context, rects);
    wxRect fieldRect = calculateFieldRect(context, rects);

    PropertyItem::redraw(dc, context);
    dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)));
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW)));
    dc.DrawRoundedRectangle(fieldRect, 3.0);

    dc.SetFont(rowFontDefault);
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    wxRect fieldTextRect = fieldRect;
    fieldTextRect.x += 4;
    fieldTextRect.width -= 5;

	/*
	dc.SetPen(*wxRED_PEN);
	dc.DrawRectangle(fieldTextRect);
	*/

	dc.SetClippingRegion(fieldTextRect);
    dc.DrawLabel(wxString(toString().c_str(), wxConvUTF8), wxBitmap(),
                 fieldTextRect, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
	dc.DestroyClippingRegion();

	
    wxRect buttonsRect = calculateButtonsRect(context, rects);
	int count = int(buttons_.size());
	int top = fieldRect.y + (fieldRect.GetHeight() - (BUTTON_SIZE + BUTTON_BORDER * 2)) / 2;
    dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW)));
	for(int i = 0; i < count; ++i){
		int left = buttonsRect.x + i * BUTTON_SPACING + BUTTON_PADDING;

		wxRect rt(left, top, BUTTON_SIZE + BUTTON_BORDER * 2, fieldRect.height);
	    dc.DrawRoundedRectangle(rt, 3.0);
		if(buttons_[i].bitmap)
			dc.DrawBitmap(*buttons_[i].bitmap, left + BUTTON_BORDER, top + BUTTON_BORDER, true);
	}
}

// ---------------------------------------------------------------------------

static const int CHECK_SIZE = 16;
static const int CHECK_PADDING = 4;

PropertyItemCheck::PropertyItemCheck(const char* name, TypeID type)
: PropertyItem(name, type)
{

}

int PropertyItemCheck::addCheckState(wxBitmap& bitmap)
{
	checkStates_.push_back(bitmap);
	return checkStates_.size() - 1;
}

#include "res/property_item_check_true.xpm"
#include "res/property_item_check_false.xpm"


void PropertyItemCheck::addDefaultCheckStates()
{
    static wxBitmap falseBitmap((const char* const*)property_item_check_false_xpm);
	VERIFY(addCheckState(falseBitmap) == 0);
    static wxBitmap trueBitmap((const char* const*)property_item_check_true_xpm);
	VERIFY(addCheckState(trueBitmap) == 1);
}

void PropertyItemCheck::calculateRects(const PropertyItem::ViewContext& context, PropertyItemRects& rects) const
{
	PropertyItem::calculateRects(context, rects);
	rects.text.x += CHECK_SIZE + CHECK_PADDING;
	rects.text.width -= CHECK_SIZE + CHECK_PADDING;
}

wxRect PropertyItemCheck::calculateCheckRect(const ViewContext& context, const PropertyItemRects& rects) const
{
	return wxRect(rects.text.x - (CHECK_SIZE + CHECK_PADDING), rects.text.y, CHECK_SIZE, CHECK_SIZE);
}

bool PropertyItemCheck::activate(const ViewContext& context)
{
	return PropertyItem::activate(context);
}

void PropertyItemCheck::redraw(wxDC& dc, const ViewContext& context) const
{
	PropertyItemRects rects;
	calculateRects(context, rects);
	wxRect checkRect = calculateCheckRect(context, rects);

	PropertyItem::redraw(dc, context);
	
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW)));

	int index = stateIndex();
	ASSERT(index >= 0 && index < int(checkStates_.size()));
    dc.DrawBitmap(checkStates_[index], checkRect.x, checkRect.y, true);
	//dc.DrawRoundedRectangle(checkRect, 3.0);
}

bool PropertyItemCheck::onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context)
{
    PropertyItemRects rects;
    calculateRects(context, rects);
    wxRect checkRect = calculateCheckRect(context, rects);

    PropertyItem::onMouseClick(pos, doubleClick, context);

    if(checkRect.Contains(pos))
		if(activate(context))
			return true;
    if(rects.text.Contains(pos) && doubleClick)
		if(activate(context))
			return true;
	return false;
}
// ---------------------------------------------------------------------------
bool PropertyItemBool::activate(const ViewContext& context)
{
	value_ = !value_;
	context.tree->commitChange(this);
	return true;
}
// ---------------------------------------------------------------------------

PropertyItemContainer::PropertyItemContainer()
: PropertyItem("")
{
}

PropertyItemContainer::PropertyItemContainer(const char* name, const ContainerSerializer& zer,
                                             bool fixedSize)
: PropertyItem(name, zer.type())
{

}

SERIALIZATION_DERIVED_TYPE(PropertyItem, PropertyItemContainer, "Container")
SERIALIZATION_FORCE_DERIVED_TYPE(PropertyItem, PropertyItemStruct)
SERIALIZATION_FORCE_DERIVED_TYPE(PropertyItem, PropertyItemFileSelector)
SERIALIZATION_FORCE_DERIVED_TYPE(PropertyItem, PropertyItemLibrarySelector)

// ---------------------------------------------------------------------------
