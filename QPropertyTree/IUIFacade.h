#pragma once

namespace property_tree {

struct IUIFacade
{
	virtual ~IUIFacade() {}
	virtual int textWidth(const char* text, Font font) = 0;
};

}
