#pragma once
#include <QObject>
#include "PropertyRow.h"

class PropertyRowFileOpen;
class QPropertyTree;

struct FileOpenMenuHandler : PropertyRowMenuHandler
{
Q_OBJECT
public:
	PropertyRowFileOpen* self;
	QPropertyTree* tree;

	FileOpenMenuHandler(PropertyRowFileOpen* self, QPropertyTree* tree)
	: self(self)
	, tree(tree)
	{
	}

public slots:
	void onMenuActivate();
	void onMenuClear();
};

std::string extractExtensionFromFilter(const char* fileSelectorFilter);
