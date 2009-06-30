#pragma once
#include "yasli/round.h"

namespace yasli{

class Archive;
struct Vect2f;
struct Vect2i{

    enum ID_init{ ID };
    enum ZERO_init{ ZERO };
    int x;
    int y;

    Vect2i()
    {
    }
    explicit Vect2i(const Vect2f&);
    Vect2i(int _x, int _y){
        set(_x, _y);
    }
    Vect2i(ID_init){
        set(1, 0);
    }
    Vect2i(ZERO_init){
        set(0, 0);
    }
    void set(int _x, int _y){
        x = _x;
        y = _y;
    }
    bool operator==(const Vect2i& rhs) const{
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Vect2i& rhs) const{
        return x != rhs.x || y != rhs.y;
    }
    Vect2i operator-() const{ return Vect2i(-x, -y); }
    Vect2i operator-(const Vect2i& rhs) const{ return Vect2i(x - rhs.x, y - rhs.y); }
    Vect2i operator+(const Vect2i& rhs) const{ return Vect2i(x + rhs.x, y + rhs.y); }
    Vect2i operator*(const Vect2i& rhs) const{ return Vect2i(x * rhs.x, y * rhs.y); }
    Vect2i operator*(int rhs) const{ return Vect2i(x * rhs, y * rhs); }
    Vect2i operator/(const Vect2i& rhs) const{ return Vect2i(x / rhs.x, y / rhs.y); }
    Vect2i operator/(int rhs) const{ return Vect2i(x / rhs, y / rhs); }
	Vect2i& operator+=(const Vect2i& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	Vect2i& operator-=(const Vect2i& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	int length2() const{ return x * x + y * y; }

    void serialize( Archive& ar );
};

inline Vect2i interpolate(Vect2i a, Vect2i b, float phase)
{
	return Vect2i(round(a.x + (b.x - a.x) * phase), round(a.y + (b.y - a.y) * phase));
}

}