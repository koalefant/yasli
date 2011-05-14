#include "stdafx.h"
#include "Canvas.h"
#include "ww/Unicode.h"
#include "ww/Win32/Handle.h"
#include "ww/Win32/Window.h" // for defaultFont

using Win32::AutoSelector;
using Win32::StockSelector;
using ww::toWideChar;

namespace ww{

Canvas::Canvas(HDC dc, HWND wnd)
: dc_(dc)
{
    RECT rt;
    GetClientRect(wnd, &rt);
    visibleRect_.set(rt.left, rt.top, rt.right, rt.bottom);
}

void Canvas::drawText(const Vect2f& pos, const char* _text, const Color4c& textColor, float xAlign, float yAlign)
{
    ESCAPE(dc_, return);
    StockSelector font(dc_, (HGDIOBJ)Win32::defaultFont());
    std::wstring text(toWideChar(_text));
    RECT rt;
    DrawText(dc_, text.c_str(), int(wcslen(text.c_str())), &rt, DT_CALCRECT);
    Vect2f size(float(rt.right - rt.left + 2), float(rt.bottom - rt.top + 2));

    Vect2f sign(-xAlign, -yAlign);

    SetBkMode(dc_, TRANSPARENT);
    SetTextColor(dc_, textColor.rgb());

    Vect2f posScr = toView(pos) + size * sign;
    TextOut(dc_, posScr.xi(), posScr.yi(), text.c_str(), wcslen(text.c_str()));
}

void Canvas::drawLine(const Vect2f& _start, const Vect2f& _end, const Color4c& color, int width)
{
    ESCAPE(dc_, return);
    Vect2i start(toView(_start));
    Vect2i end(toView(_end));

    AutoSelector pen(dc_, CreatePen(PS_SOLID, width, color.rgb()));
    MoveToEx(dc_, start.x, start.y, 0);
    LineTo(dc_, end.x, end.y);
}

void Canvas::fillCircle(const Vect2f& pos, float radius, const Color4c& color)
{
    ESCAPE(dc_, return);
    Vect2i p(toView(pos));
    Vect2f center = toView(pos);
    Recti r(toView(pos - Vect2f(radius, radius)),
            toView(pos + Vect2f(radius, radius)));
    AutoSelector oldPen(dc_, CreateSolidBrush(RGB(color.r, color.g, color.b)));
    AutoSelector oldBrush(dc_, GetStockObject(NULL_PEN));
    Ellipse(dc_, r.left(), r.top(), r.right(), r.bottom());
}

void Canvas::fillRectangle(const Rectf& rect, const Color4c& color)
{
    Vect2i leftTop(toView(rect.leftTop()));
    Vect2i rightBottom(toView(rect.rightBottom()));
    if(leftTop.y > rightBottom.y)
        std::swap(leftTop.y, rightBottom.y);
    RECT rt = { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
    Win32::Handle<HBRUSH> brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
    FillRect(dc_, &rt, brush);
}

Vect2i Canvas::toView(const Vect2f& p) const
{
    return Vect2i(p);
}

Vect2f Canvas::fromView(const Vect2i& p) const
{
    return Vect2f(p);
}

Rectf Canvas::visibleArea() const
{
    return Rectf(fromView(Vect2i(visibleRect_.left(), visibleRect_.top())),
                 fromView(Vect2i(visibleRect_.right(), visibleRect_.bottom())));
}

// ---------------------------------------------------------------------------
static int sysColorToWinColor(SysColor color)
{
    switch(color)
    {
    case SYSCOLOR_HIGHLIGHT: return COLOR_HIGHLIGHT;
    case SYSCOLOR_BTNFACE: return COLOR_BTNFACE;
    case SYSCOLOR_BTNSHADOW: return COLOR_BTNSHADOW;
    default: return COLOR_BTNFACE;
    }    
};

Color4c sysColor(SysColor color)
{
    Color4c result;
    result.setGDI(GetSysColor(sysColorToWinColor(color)));
    return result;
}

}
