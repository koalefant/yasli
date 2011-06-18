#include "StdAfx.h"

#include "ww/Widget.h"
#include "ww/Container.h"
#include "ww/Window.h"

#include "ww/Win32/Window.h"

#include "ww/Serialization.h"

namespace ww{

Widget::Widget()
: border_(0)
, minimalSize_(0, 0)
, requestSize_(0, 0)
, position_(0, 0, 1, 1)
, parent_(0)
, visible_(false)
, sensitive_(true)
, visibleInLayout_(true)
, relayoutQueued_(true)
, toolTip_(0)
{
}

Widget::~Widget()
{
	signalDelete_.emit();
    if(_focusedWidget() == this)
        _setFocusedWidget(0);
}

void Widget::show()
{
	bool wasVisible = visible_;
	visible_ = true;
	if(!wasVisible)
		_updateVisibility();
}

void Widget::hide()
{
	visible_ = false;
	_updateVisibility();
}

void Widget::setVisibility(bool visible)
{
	if(visible)
		show();
	else
		hide();
}

void Widget::_setVisibleInLayout(bool visibleInLayout)
{
	bool wasVisibleInLayout = visibleInLayout_;
	visibleInLayout_ = visibleInLayout;
	if(wasVisibleInLayout != visibleInLayout_)
		_updateVisibility();
}

void Widget::_setPosition(const ww::Rect& position)
{
	position_ = position;
}

void Widget::setRequestSize(const Vect2 size)
{
	requestSize_ = size;
}

void Widget::setRequestSize(int w, int h)
{
	requestSize_.set(w, h);
}

void Widget::setBorder(int border)
{
	border_ = border;
}

const Vect2 Widget::_minimalSize() const
{ 
	return Vect2(max(minimalSize_.x, requestSize_.x), max(minimalSize_.y, requestSize_.y)); 
}

void Widget::_setMinimalSize(int w, int h)
{
	_setMinimalSize(Vect2(w, h));
}

void Widget::_setMinimalSize(const Vect2& size)
{
	minimalSize_ = size;
}

void Widget::_setParent(Container* parent)
{
	parent_ = parent;
}

void Widget::_queueRelayout()
{
	if(!isVisible()){
		relayoutQueued_ = true;
		if(parent())
			parent()->_queueRelayout();
	}
	else
		_relayoutParents();
}

void Widget::_relayoutParents(bool propagate)
{
	relayoutQueued_ = false;
	if(propagate && parent()){
		if(parent()->isVisible())
			parent()->_relayoutParents();
		else
			parent()->_queueRelayout();
	}
}

void Widget::_relayoutParents()
{
	_relayoutParents(true);
}

void Widget::_updateVisibility()
{
	if(visible_ && relayoutQueued_){
		_relayoutParents();
	}
}

void Widget::setFocus()
{
    //ESCAPE(_focusable(), return);
    if(_focusable() && _window())
        ::SetFocus(*_window());
}

HotkeyContext* Widget::_hotkeyContext() 
{ 
	return parent_ ? parent_->_hotkeyContext() : 0; 
}

Widget* Widget::_focusedWidget() 
{ 
	return parent_ ? parent_->_focusedWidget() : 0; 
}

Widget* Widget::_nextWidget(Widget* last, FocusDirection direction) const
{
	return 0;
}

Widget* Widget::_nextFocusableWidget(Widget* last, FocusDirection direction) const
{
	switch(direction){
	case FOCUS_FIRST:
		if(ww::Widget* widget = _nextWidget(last, direction)){
			if(!widget->_focusable())
				return _nextFocusableWidget(widget, FOCUS_NEXT);
		}
		else
			return 0;
	case FOCUS_LAST:
		if(ww::Widget* widget = _nextWidget(last, direction)){
			if(!widget->_focusable())
				return _nextFocusableWidget(widget, FOCUS_PREVIOUS);
		}
		else
			return 0;
	case FOCUS_NEXT:
	case FOCUS_PREVIOUS:
	{
		ww::Widget* widget = last;
		while(widget = _nextWidget(widget, direction))
			if(widget->_focusable())
				return widget;
        return 0;
	}
	}
	return 0;
}

void Widget::passFocus(FocusDirection direction)
{
	Widget* focused = _focusedWidget();
    if(!focused)
        focused = this;
	Widget* next = 0;
	Widget* container = focused->parent();

	if(container){
		do{
			if(!container)
				break;

			next = container->_nextFocusableWidget(focused, direction);
			if(next){
				ASSERT(next->_focusable());
				if(next->_focusable()){
					next->_setFocus();
					break;
				}
				else{
					focused = 0;
					container = next;
					next = 0;
				}
			}
			else{
				focused = container;
				if(container->parent()){
					container = container->parent();
				}
				else{
					container = container->parent();
				}
			}
		} while(next == 0);
	}
}

void Widget::_setFocus()
{
    if(_focusable() && _focusedWidget() != this)
        _setFocusedWidget(this);
}

void Widget::_setFocusedWidget(Widget* widget)
{
	if(parent_) 
		parent_->_setFocusedWidget(widget); 
}

struct ShowAllVisitor : public WidgetVisitor{
	void operator()(Widget& widget){
		widget.showAll();
	}
};

void Widget::showAll()
{
	visitChildren(ShowAllVisitor());
	show();
}

void Widget::serialize(Archive& ar)
{
	if(ar.filter(ww::SERIALIZE_DESIGN)){
		/*
		ar.serialize(_property(border_, this, &Widget::setBorder), "border", "Отступы");
		ar.serialize(requestSize_, "requestSize", "Размер");
		ar.serialize(_property(sensitive_, this, &Widget::setSensitive), "sensitive", "Активность");
		ar.serialize(_property(visible_, this, &Widget::setVisibility), "visible", "Видимость");
		*/
	}
}

// --------------------------------------------------------------------------------

void Container::setBorder(int border)
{
	Widget::setBorder(border);
	_arrangeChildren();
}

bool Container::_focusable() const
{
	return _nextFocusableWidget(0, FOCUS_FIRST) != 0;
}

void Container::_setFocus()
{
	ASSERT(_focusable());
	ww::Widget* widget = _nextFocusableWidget(0, FOCUS_FIRST);
	ASSERT(widget != 0);
	if(widget)
		widget->_setFocus();
}

struct FocusMover : public WidgetVisitor{
	FocusMover(Widget* current, FocusDirection direction)
	: direction_(direction)
	, current_(current)
	, previous_(0)
	, result_(0)
	{}

	Widget* result(){ return result_; }
	void operator()(Widget& widget){
		//if(widget._focusable()){
		switch(direction_){
		case FOCUS_FIRST:
			if(!result_)
				result_ = &widget;
			break;
		case FOCUS_LAST:
			result_ = &widget;
			break;
		case FOCUS_NEXT:
			if(previous_ == current_)
				result_ = &widget;
			break;
		case FOCUS_PREVIOUS:
			if(&widget == current_)
				result_ = previous_;
			break;
		}
		previous_ = &widget;
		// }
	}
protected:
	FocusDirection direction_;
	Widget* result_;
	Widget* previous_;
	Widget* current_;
};

Widget* Container::_nextWidget(Widget* last, FocusDirection direction) const
{
	switch(direction){
	case FOCUS_DOWN:
	case FOCUS_RIGHT:
		direction = FOCUS_NEXT;
		break;
	case FOCUS_UP:
	case FOCUS_LEFT:
		direction = FOCUS_PREVIOUS;
		break;
	default:
		break;
	}
	FocusMover mover(last, direction);
	visitChildren(mover);
	return mover.result();
}

bool Container::isActive() const
{
	Win32::Window32* window = _findWindow(this);
	if(window){
		// получаем родительское окно и смотрим активно ли оно
		while(window->parent())
			window = window->parent();
		return (::GetActiveWindow() == *window);
	}
	else{
		ASSERT(0);
		return false;
	}
}

bool Container::isVisible() const
{
	return parent() ? parent()->isVisible() : false;
	/*
	if(Win32::Window32* window = _findWindow())
		return ::IsWindowVisible(*window) ? TRUE : FALSE;
	return false;
	*/
}

Win32::Window32* _findWindow(const Widget* widget)
{
	while(widget){
		if(Win32::Window32* window = widget->_window())
			return window;
		else
			widget = widget->parent();
	}
	return Win32::_globalDummyWindow;
}

void _ensureChildren(ww::Container* container, ww::Widget* widget)
{
	if(widget && container && widget->parent() != container){
		container->_arrangeChildren();
		widget->_setParent(container);
		container->_queueRelayout();
	}
}

}
