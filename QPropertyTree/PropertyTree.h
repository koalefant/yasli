#pragma once

namespace property_tree {

struct IUIFacade;

class PropertyTree
{
protected:
	PropertyTree(IUIFacade* ui) : ui_(ui) {}
public:
	IUIFacade* ui() const { return ui_; }

private:
	IUIFacade* ui_;
};

}
