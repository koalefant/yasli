/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


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
