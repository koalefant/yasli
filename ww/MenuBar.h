#pragma once
#include "ww/_WidgetWithWindow.h"
#include "ww/KeyPress.h"
#include "ww/sigslot.h"

namespace ww{

class HotkeyContext;
class MenuBarImpl;
class Window;

class WW_API MenuItem : public RefCounter, public sigslot::has_slots
{
public:
    MenuItem(const char* text)
    : text_(text)
    , commandIndex_(0)
    {
    }

    MenuItem& add(const char* text);

    void setText(const char* text);
    const char* text() const{ return text_.c_str(); }
    void setHotkey(const KeyPress &hotkey){ hotkey_ = hotkey; }
    const KeyPress& hotkey() const{ return hotkey_; }

    typedef sigslot::signal0 SignalActivate;
    SignalActivate& signalActivate(){ return signalActivate_; }
    typedef sigslot::signal1<MenuItem*> SignalUpdate;
    SignalUpdate& signalUpdate(){ return signalUpdate_; }
    void activate(){
        signalActivate_.emit();
    }

    void registerHotkeys(HotkeyContext *context);
private:
    MenuItem* findSubItem(const char* name);

    std::string text_;
    SignalActivate signalActivate_;
    SignalUpdate signalUpdate_;
    KeyPress hotkey_;

    int commandIndex_;
    typedef std::vector<SharedPtr<MenuItem> > Items;
    Items subItems_;
    friend MenuBarImpl;
};

class WW_API MenuBar : public _WidgetWithWindow{
public:
    MenuBar(int border = 0);
    ~MenuBar();

    MenuItem& add(const char* text);

    template<class T>
    MenuItem& add(const char* text, T* obj, void(T::*_handler)(void)){
        MenuItem &result = add(text);
        result.signalActivate().connect(obj, _handler);
        return result;
    }

    void registerHotkeys(Window* mainWindow);

    void _setPosition(const Rect& position);
    void _setParent(Container* container);
    void _updateVisibility();
protected:
    friend class MenuBarImpl;
    MenuBarImpl* impl();
};

}

