#pragma once

#include <vector>
#include "ww/_WidgetWithWindow.h"
#include "ww/VBox.h"

namespace ww{

class TabsImpl;

class TabChanger{};

class Tabs : public _WidgetWithWindow, public TabChanger{
public:
	Tabs(int border = 0);

	int add(const char* tabTitle, int before = -1);
	void remove(int index);
	void clear();

	int selectedTab();
	void setSelectedTab(int index, const TabChanger* changer = 0);

	typedef sigslot::signal1<const TabChanger*> SignalChanged;
	SignalChanged& signalChanged(){ return signalChanged_; }
	
	typedef sigslot::signal2<MouseButton, int> SignalMouseButtonDown;
	SignalMouseButtonDown& signalMouseButtonDown(){ return signalMouseButtonDown_; }

	void serialize(yasli::Archive& ar);

protected:
	SignalChanged signalChanged_;
	SignalMouseButtonDown signalMouseButtonDown_;

	TabsImpl& impl();
	friend class TabsImpl;
};

class TabPages : public VBox, public TabChanger{
public:
	TabPages(int border = 0);

	int add(const char* title, Widget* widget, int before = -1);
	void remove(int index);
	Widget* widgetByIndex(int index);
	int size() const;

	int selectedTab() { return tabs_->selectedTab(); }
	void setSelectedTab(int index);

	void serialize(yasli::Archive& ar);

	typedef sigslot::signal1<const TabChanger*> SignalChanged;
    SignalChanged &signalChanged(){ return tabs_->signalChanged(); }
protected:
	void onTabChange(const TabChanger* changer);
	yasli::SharedPtr<Tabs> tabs_;
	typedef std::vector<yasli::SharedPtr<Widget> > Widgets;
	Widgets widgets_;
};

}

