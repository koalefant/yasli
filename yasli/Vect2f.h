#pragma once
#include <math.h>

namespace yasli{

class Archive;
struct Vect2i;
struct Vect2f{

    enum ID_init{ ID };
    enum ZERO_init{ ZERO };
    float x;
    float y;

    Vect2f()
    {
    }
    explicit Vect2f(const Vect2i &value);
    Vect2f(float _x, float _y){
        set(_x, _y);
    }
    Vect2f(ID_init){
        set(1, 0);
    }
    Vect2f(ZERO_init){
        set(0, 0);
    }
    void set(float _x, float _y){
        x = _x;
        y = _y;
    }
    bool operator==(const Vect2f& rhs) const{
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Vect2f& rhs) const{
        return x != rhs.x || y != rhs.y;
    }
    Vect2f operator-() const{ return Vect2f(-x, -y); }
    Vect2f operator-(const Vect2f& rhs) const{ return Vect2f(x - rhs.x, y - rhs.y); }
    Vect2f operator+(const Vect2f& rhs) const{ return Vect2f(x + rhs.x, y + rhs.y); }
    Vect2f operator*(const Vect2f& rhs) const{ return Vect2f(x * rhs.x, y * rhs.y); }
    Vect2f operator*(float rhs) const{ return Vect2f(x * rhs, y * rhs); }
    Vect2f operator/(const Vect2f& rhs) const{ return Vect2f(x / rhs.x, y / rhs.y); }
    Vect2f operator/(float rhs) const{ return Vect2f(x / rhs, y / rhs); }
	Vect2f& operator+=(const Vect2f& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	Vect2f& operator-=(const Vect2f& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	float length2() const{ return x * x + y * y; }
	float length() const{ return sqrtf(x * x + y * y); }

    void serialize( Archive& ar );
};

}