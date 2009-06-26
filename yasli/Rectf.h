#pragma once

#ifdef min
# undef min
#endif

#ifdef max
# undef max
#endif
#include "Vect2f.h"

struct Rectf{
public:
    Rectf()
    {}
    Rectf(Vect2f _min, Vect2f _max)
    : min(_min), max(_max) {}
    Rectf(float minX, float minY, float maxX, float maxY){
        set(minX, minY, maxX, maxY);
    }
    void set(Vect2f _min, Vect2f _max){
        min = _min;
        max = _max;
    }
    void set(float minX, float minY, float maxX, float maxY){
        min.x = minX;
        min.y = minY;
        max.x = maxX;
        max.y = maxY;
    }
    Vect2f size() const{ return max - min; }
    bool isValid() const{
        return min.x <= max.x && min.y <= max.y;
    }
    void validate(){
        if(min.x > max.x)
            std::swap(min.x, max.x);
        if(min.y > max.y)
            std::swap(min.x, max.x);
    }
	Rectf operator+(Vect2f offset){
		return Rectf( min + offset, max + offset );
	}

    Vect2f min;
    Vect2f max;
	Vect2f center() const{ return (min + max) / 2; }

    void setLeft( float _left ){ min.x = _left; }
    void setTop( float _top ){ min.y = _top; }
    void setRight( float _right ){ max.x = _right; }
    void setBottom( float _bottom ){ max.y = _bottom; }
    void setWidth( float _width ){ max.x = min.x + _width; }
    void setHeight( float _height ){ max.y = min.y + _height; }
	float left() const{ return min.x; }
	float top() const{ return min.y; }
	float right() const{ return max.x; }
	float bottom() const{ return max.y; }
	float width() const{ return max.x - min.x; }
	float height() const{ return max.y - min.y; }

    bool pofloatInside(Vect2f _pofloat) const
    {
      if ( _pofloat.x < min.x || _pofloat.x > max.x )
        return false;
      if ( _pofloat.y < min.y || _pofloat.y > max.y )
        return false;
      return true;
    }

    void serialize(Archive& ar);
};
