/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "PropertyTree/PropertyTreeBase.h"
#include "PropertyTree/ConstStringList.h"
#include "ww/_WidgetWithWindow.h"
#include "yasli/Serializer.h"
#include "yasli/Object.h"
#include "ww/Strings.h"
#include <vector>

namespace Gdiplus {
	class Font;
	class Graphics;
};

namespace yasli {
	class Object;
};

namespace property_tree {
	struct KeyEvent;
};

class PropertyTreeOperator;
class PropertyTreeModel;
class PropertyRow;

struct HDC__;
typedef HDC__* HDC;

namespace ww {
using std::vector;
struct Color;
class DragController;
class DragWindow;
class TreeImpl;
class PopupMenuItem;
typedef std::vector<yasli::SharedPtr<PropertyRow> > PropertyRows;
class Entry;

class PropertyTree : public _ContainerWithWindow, public PropertyTreeBase
{
public:
	PropertyTree(int border = 0);
	~PropertyTree();

	void attachPropertyTree(PropertyTreeBase* propertyTree) override;

	void serialize(Archive& ar);

	void update(); // needs to be called after model change

	bool hasFocus() const;
	bool hasFocusOrInplaceHasFocus() const override;

	// internal methods:
	Vect2 _toScreen(Vect2 point) const;
	void _setFocus();
	void _drawRowLabel(Gdiplus::Graphics* gr, const char* text, Gdiplus::Font* font, const Rect& rect, const Color& color) const;
	void _drawRowValue(Gdiplus::Graphics* gr, const char* text, Gdiplus::Font* font, const Rect& rect, const Color& color, bool pathEllipsis, bool center) const;
	Rect _visibleRect() const;
	Gdiplus::Graphics* _graphics() const { return graphics_; }
	TreeImpl* impl() const;

	Signal<>& signalChanged(){ return signalChanged_; }
	typedef Signal<const yasli::Object&> SignalObjectChanged;
	SignalObjectChanged& signalObjectChanged(){ return signalObjectChanged_; }
	typedef Signal<yasli::Archive&> SignalSerialized;
	SignalSerialized& signalSerialized(){ return signalSerialized_; }
	Signal<>& signalSelected(){ return signalSelected_; }
	Signal<>& signalReverted(){ return signalReverted_; }
	Signal<>& signalPushUndo(){ return signalPushUndo_; }

protected:
	void onChanged() override { signalChanged_.emit(); }
	void onContinuousChange() override { signalContinuousChange_.emit(); }
	void onAboutToSerialize(yasli::Archive& ar) { signalAboutToSerialize_.emit(ar); }
	void onSerialized(yasli::Archive& ar) { signalSerialized_.emit(ar); }
	void onSelected() override { signalSelected_.emit(); }
	void onReverted() override { signalReverted_.emit(); }
	void onPushUndo() override { signalPushUndo_.emit(); }

	void onPaint(HDC dc);
	void onLButtonUp(int button, int x, int y);
	void onLButtonDoubleClick(int x, int y);
	void onLButtonDown(int button, int x, int y);
	void onRButtonDown(int button, int x, int y);
	void onMouseMove(int button, int x, int y);
	void onMouseWheel(int delta);
	void onAttachedTreeChanged();
	void onScroll(int y);

	void repaint() override { update(); }
	void updateHeights() override;
	bool updateScrollBar();
	void defocusInplaceEditor() override;
	int filterAreaHeight() const override;

	void onFilterChanged();

	property_tree::Point offset() const { return offset_; }
	property_tree::Rect area() const { return area_; }

	void copyRow(PropertyRow* row) override;
	void pasteRow(PropertyRow* row) override;
	bool canBePasted(PropertyRow* destination) override;
	bool canBePasted(const char* destinationType) override;

	void setFilterMode(bool inFilterMode);
	void startFilter(const char* filter) override;
	void _arrangeChildren() override;
	void visitChildren(WidgetVisitor& visitor) const;
	void resetFilter() override;
	int tabSize() const;

	void drawFilteredString(Gdiplus::Graphics* gr, const char* text, RowFilter::Type type, Gdiplus::Font* font, const Rect& rect, const Color& color, bool pathEllipsis, bool center) const;
	void interruptDrag();

	Signal<> signalChanged_;
	Signal<> signalContinuousChange_;
	SignalObjectChanged signalObjectChanged_;
	Signal<> signalReverted_;
	Signal<> signalSelected_;
    Signal<> signalPushUndo_;
	Signal<yasli::Archive&> signalAboutToSerialize_;
	SignalSerialized signalSerialized_;

	Vect2 pressPoint_;

	std::unique_ptr<DragController> dragController_;

	PolyPtr<Entry> filterEntry_; 
	Gdiplus::Graphics* graphics_;

	friend class TreeImpl;
	friend class DragController;
	friend class DragWindow;
	friend class FilterEntry;
	friend struct FilterVisitor;
};

}

// vim:ts=4
