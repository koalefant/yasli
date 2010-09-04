#pragma once

#include <string>
#include "ww/Container.h"
#include "ww/HotkeyContext.h"
#include "Win32/Types.h"

namespace Win32{
    class Window;
};

namespace ww{

class WindowImpl;

enum WindowPosition{
	POSITION_DEFAULT,
	POSITION_CENTER,
	//POSITION_CENTER_PARENT,
	POSITION_LEFT_TOP,
	POSITION_LEFT,
	POSITION_LEFT_BOTTOM,
	POSITION_RIGHT_TOP,
	POSITION_RIGHT,
	POSITION_RIGHT_BOTTOM,
	POSITION_TOP,
	POSITION_BOTTOM,
	POSITION_CURSOR
};


class WW_API Window : public Container{
public:
    Window(Application* app, int border = 4);
    //Window(Widget* owner, int border = 4);
    Window(int border = 4, int style = 0);

    // virtuals:
    ~Window();

	bool isVisible() const;
	void showAll();

    void visitChildren(WidgetVisitor& visitor) const;
    // ^^^


	/// добавить контрол в окно
	void add(Widget* widget);
    /// убирает единственный дочерний виджет
    void remove();

	/// устанавливает заголок окна (тот, что в TitleBar-е)
	void setTitle(std::string str);

	void setShowTitleBar(bool showTitleBar);
	void setIconFromResource(const char* resourceName);
	void setIconFromFile(const char* resourceName);

	void setDefaultPosition(WindowPosition position);
	void setDefaultSize(Vect2i size);

    /// разрешает/запрещает изменение размеров окна
	void setResizeable(bool allowResize);
	bool resizeable() const{ return resizeable_; }
    /// разрешает/запрещает сворачивание окна
	void setMinimizeable(bool allowMinimize);
	bool minimizeable() const{ return minimizeable_; }

	Recti restoredPosition() const;
	void setRestoredPosition(const Recti& position);

	void setMaximized(bool maximized);
	bool maximized() const;

	// signals
	sigslot::signal0& signalClose(){ return signalClose_; }
    sigslot::signal0& signalActivate(){ return signalActivate_; }

	// virtual events:
	virtual void onClose();
	virtual void onResize(int width, int height) {}
	virtual void onSetFocus() {}

	void serialize(Archive& ar);

	Win32::Window32* _window() const{ return window_; }

	void setDefaultWidget(Widget* widget);

	sigslot::signal0& signalPressed(KeyPress key) { return hotkeyContext_->signalPressed(key); }
	sigslot::signal2<KeyPress, bool&>& signalPressedAny(){ return hotkeyContext_->signalPressedAny(); }

	void onHotkeyFocusNext();
	void onHotkeyFocusPrev();

	HotkeyContext* _hotkeyContext(){ return hotkeyContext_; }
    void _setFocusedWidget(Widget* widget);
    Widget* _focusedWidget(){ return focusedWidget_; }

protected:
    Window(HWND parent, int border);
	void init(HWND parent, int border, Application* app);

	void _updateVisibility();
	void _arrangeChildren();
	void _relayoutParents();

	void _setFocus();
	unsigned int calculateStyle();
	void reposition();


	sigslot::signal0 signalClose_;
	sigslot::signal0 signalActivate_;

	bool resizeable_;
	bool minimizeable_;
	bool showTitleBar_;
	bool positioned_;
	WindowPosition windowPosition_;
	Vect2i defaultSize_;
	std::string title_;
	int style_;

	yasli::SharedPtr<Widget> child_;
	Widget* focusedWidget_;
	yasli::SharedPtr<HotkeyContext> hotkeyContext_;

    Win32::Window32* window_;

	Widget* defWidget_;
    Application* app_;

    friend class WindowImpl;
};

};

