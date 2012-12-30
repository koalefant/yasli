#pragma once

#include "PropertyRow.h"

struct PropertyTreeMenuHandler : PropertyRowMenuHandler
{
	Q_OBJECT
public:
	PropertyRow* row;
	QPropertyTree* tree;

	yasli::string filterName;
	yasli::string filterValue;
	yasli::string filterType;

public slots:
	void onMenuFilterByName();
	void onMenuFilterByValue();
	void onMenuFilterByType();

	void onMenuUndo();

	void onMenuCopy();
	void onMenuPaste();
};
