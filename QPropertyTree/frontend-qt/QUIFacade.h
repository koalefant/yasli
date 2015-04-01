#pragma once

#include "../IDrawContext.h"
#include "../IUIFacade.h"

class QPropertyTree;

namespace property_tree {

class QUIFacade : public IUIFacade
{
public:
	QUIFacade(QPropertyTree* tree) : tree_(tree) {}

	int textWidth(const char* text, Font font) override;
private:
	QPropertyTree* tree_;
};

}
