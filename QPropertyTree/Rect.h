#pragma once

#include <QPoint>
#include <QRect>

namespace property_tree {

struct Point
{
	Point() {}
	Point(int x, int y) : x_(x), y_(y) {}

	Point operator+(const Point& p) const { return Point(x_ + p.x_, y_ + p.y_); }
	Point& operator+=(const Point& p) { *this = *this + p; return *this; }
	Point(const QPoint& rect);
	operator QPoint() const;

	int x_;
	int y_;

	// weird accessors to mimic QPoint
	int x() const { return x_; }
	int y() const { return y_; }
	void setX(int x) { x_ = x; }
	void setY(int y) { y_ = y; }
};

struct Rect
{
	int x;
	int y;
	int w;
	int h;

	int left() const { return x; }
	int top() const { return y; }
	int right() const { return x + w; }
	int bottom() const { return y + h; }

	int width() const { return w; }
	int height() const { return h; }

	Rect() : x(0), y(0), w(0), h(0) {}
	Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

	Rect(const QRect& rect);
	operator QRect() const;

	template<class TPoint>
	bool contains(const TPoint& p) const {
		if (p.x() < x || p.x() >= x + w)
			return false;
		if (p.y() < y || p.y() >= y + h)
			return false;
		return true;
	}

	Rect adjusted(int l, int t, int r, int b) const {
		return Rect(x + l, y + t,
			x + w + r - (x + l),
			y + h + b - (y + t));
	}
};

}

using property_tree::Rect; // temporary
using property_tree::Point; // temporary
