#pragma once
#include "sigslot.h"
#include "Rect.h"

namespace property_tree {

enum
{
	MENU_DISABLED = 1 << 0,
	MENU_DEFAULT = 1 << 1
};

class IMenuAction
{
public:
	sigslot::signal0 signalTriggered;
};

class IMenu
{
public:
	virtual ~IMenu() {}
	virtual bool isEmpty() = 0;
	virtual IMenu* addMenu(const char* text) = 0;
	virtual IMenu* findMenu(const char* text) = 0;
	virtual void addSeparator() = 0;
	virtual IMenuAction* addAction(const char* text, int flags = 0) = 0;
	virtual void exec(const Point& point) = 0;

	template<class T>
	void addAction(const char* text, const char* shortcut, int flags, T* obj, void(T::*handler)())
	{
		IMenuAction* command = addAction(text, flags);
		command->signalTriggered.connect(obj, handler);
	}

	template<class T>
	void addAction(const char* text, int flags, T* obj, void(T::*handler)())
	{
		IMenuAction* command = addAction(text, flags);
		command->signalTriggered.connect(obj, handler);
	}

};
}