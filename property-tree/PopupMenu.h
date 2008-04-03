#pragma once
#include "yasli/sigslot.h"
#include <wx/menu.h>

class wxWindow;

class PopupMenuItem0;
template<class Arg1> class PopupMenuItem1;
template<class Arg1, class Arg2> class PopupMenuItem2;

class PopupMenuItem : public RefCounter{
public:
    PopupMenuItem(const char* text = "");
    virtual ~PopupMenuItem() {}
    PopupMenuItem0& add(const char* text);
	template<class Arg1> PopupMenuItem1<Arg1>& add(const char* text, const Arg1& arg1);
	template<class Arg1, class Arg2> PopupMenuItem2<Arg1, Arg2>& add(const char* text, const Arg1& arg1, const Arg2& arg2);
    void addSeparator();
    PopupMenuItem& setSensitive(bool sensitive){ sensitive_ = sensitive; return *this; }
    bool sensitive() const{ return sensitive_; }
    typedef std::vector<SharedPtr<PopupMenuItem> > Children;
    Children& children(){ return children_; }
    const Children& children() const{ return children_; }
    bool empty() const{ return children_.empty(); }
    const char* text() const{ return text_.c_str(); }
    virtual void invoke() = 0;
protected:
    PopupMenuItem& add(PopupMenuItem* item);
    std::string text_;
    bool sensitive_;

    Children children_;
};

class PopupMenuItem0 : public PopupMenuItem{
public:
    PopupMenuItem0(const char* text = "")
    : PopupMenuItem(text)
    {}
    template<class T>
    PopupMenuItem0& connect(T* object, void(T::*member)(void)){
        signal_.connect(object, member);
        return *this;
    }
    void invoke(){
        signal_.emit();
    }
protected:
    sigslot::signal0<> signal_;
};

template<class Arg1>
class PopupMenuItem1 : public PopupMenuItem{
public:
    PopupMenuItem1(const char* text, const Arg1& arg1)
    : PopupMenuItem(text)
	, arg1_(arg1)
    {}
    template<class T, class _Arg1>
    PopupMenuItem1& connect(T* object, void(T::*member)(_Arg1)){
        signal_.connect(object, member);
        return *this;
    }
    void invoke(){
        signal_.emit(arg1_);
    }
protected:
    sigslot::signal1<Arg1> signal_;
	Arg1 arg1_;
};


template<class Arg1, class Arg2>
class PopupMenuItem2 : public PopupMenuItem{
public:
    PopupMenuItem2(const char* text, const Arg1& arg1, const Arg2& arg2)
    : PopupMenuItem(text)
	, arg1_(arg1)
	, arg2_(arg2)
    {}
    template<class T>
    PopupMenuItem2& connect(T* object, void(T::*member)(Arg1, Arg2)){
        signal_.connect(object, member);
        return *this;
    }
    void invoke(){
        signal_.emit(arg1_, arg2_);
    }
protected:
    sigslot::signal2<Arg1, Arg2> signal_;
	Arg1 arg1_;
	Arg2 arg2_;
};

inline PopupMenuItem0& PopupMenuItem::add(const char* text){
    return static_cast<PopupMenuItem0&>(add(new PopupMenuItem0(text)));
}

template<class Arg1>
PopupMenuItem1<Arg1>& PopupMenuItem::add(const char* text, const Arg1& arg1){
    return static_cast<PopupMenuItem1<Arg1>&>(add(new PopupMenuItem1<Arg1>(text, arg1)));
}

template<class Arg1, class Arg2>
PopupMenuItem2<Arg1, Arg2>& PopupMenuItem::add(const char* text, const Arg1& arg1, const Arg2& arg2){
    return static_cast<PopupMenuItem2<Arg1, Arg2>&>(add(new PopupMenuItem2<Arg1, Arg2>(text, arg1, arg2)));
}

enum{
    ID_FIRST_COMMAND = 10000,
    ID_LAST_COMMAND = 20000
};

class PopupMenu : protected wxMenu{
public:
    PopupMenuItem& root(){ return root_; }

    void spawn(wxWindow* window, int x = -1, int y = -1);

    DECLARE_CLASS(PopupMenu)
    DECLARE_EVENT_TABLE()
protected:
    void generateMenu(wxMenu* menu, wxMenu* subMenu, PopupMenuItem& item);

    void onMenuEvent(wxCommandEvent& event);

    typedef std::vector<PopupMenuItem*> Handlers;
    Handlers handlers_;

    PopupMenuItem0 root_;
};
