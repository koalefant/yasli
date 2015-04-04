#pragma once

#include "../IDrawContext.h"
#include "../IUIFacade.h"
#include "../IMenu.h"
#include <QMenu>

class QPropertyTree;

namespace property_tree {

class QtAction : public IMenuAction
{
public:
	QtAction(QAction* action)
	{
	}
private:
	QAction* action_;
};

class QtMenu : public QObject, public IMenu
{
	Q_OBJECT
public:
	QtMenu(const char* text, QPropertyTree* tree)
	: menu_(text)
	, tree_(tree)
	{
	}

	bool isEmpty() override { return menu_.isEmpty(); }
	IMenu* addMenu(const char* text) { 
		menus_.push_back(new QtMenu(text, tree_));
		return menus_.back();
	}
	IMenu* findMenu(const char* text) {
		return 0;
	}
	void addSeparator() { menu_.addSeparator(); }
	IMenuAction* addAction(const char* text, int flags = 0) {
		QAction* qaction = menu_.addAction(text);
		QtAction* qtAction = new QtAction(qaction);
		actions_.push_back(qtAction);
		return qtAction;
	}
	
	void exec(const Point& point) {
	}
private:
	std::vector<QtAction*> actions_;
	std::vector<QtMenu*> menus_;
	QMenu menu_;
	QPropertyTree* tree_;
};

class QUIFacade : public IUIFacade
{
public:
	QUIFacade(QPropertyTree* tree) : tree_(tree) {}

	IMenu* createMenu() override;
	void setCursor(Cursor cursor) override;
	void unsetCursor() override;
	Point cursorPosition() override;

	Point screenSize() override;
	int textWidth(const char* text, Font font) override;
private:
	QPropertyTree* tree_;
};

}
