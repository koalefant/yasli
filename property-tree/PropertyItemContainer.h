#pragma once
#include "PropertyItem.h"

class ContainerSerializer;
class PropertyItemContainer : public PropertyItem, public PropertyWithButtons{
public:
	using PropertyItem::ViewContext;
    PropertyItemContainer();
    PropertyItemContainer(const char* name, const ContainerSerializer& zer,
                          bool fixedSize);

    // from PropertyItem:
    bool isContainer() const{ return true; }
	bool activate(const ViewContext& context);
    void redraw(wxDC& dc, const ViewContext& context) const;
    bool onMouseClick(wxPoint pos, bool doubleClick, const ViewContext& context);
    // ^^^
protected:
    // from PropertyWithButtons:
	bool onButton(int index, const ViewContext& context);
    // ^^^

    TypeID childrenType_;
    bool fixedSize_;
};
