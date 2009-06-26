#pragma once

#ifdef min
# undef min
#endif

#ifdef max
# undef max
#endif
#include "Vect2i.h"

struct Recti{
public:
    Recti()
    {}
    Recti(Vect2i _min, Vect2i _max)
    : min(_min), max(_max) {}
    Recti(int minX, int minY, int maxX, int maxY){
        set(minX, minY, maxX, maxY);
    }
    void set(Vect2i _min, Vect2i _max){
        min = _min;
        max = _max;
    }
    void set(int minX, int minY, int maxX, int maxY){
        min.x = minX;
        min.y = minY;
        max.x = maxX;
        max.y = maxY;
    }
    Vect2i size() const{ return max - min; }
    bool isValid() const{
        return min.x <= max.x && min.y <= max.y;
    }
    void validate(){
        if(min.x > max.x)
            std::swap(min.x, max.x);
        if(min.y > max.y)
            std::swap(min.x, max.x);
    }
	Recti operator+(Vect2i offset){
		return Recti( min + offset, max + offset );
	}

    Vect2i min;
    Vect2i max;
	Vect2i center() const{ return (min + max) / 2; }

    void setLeft( int _left ){ min.x = _left; }
    void setTop( int _top ){ min.y = _top; }
    void setRight( int _right ){ max.x = _right; }
    void setBottom( int _bottom ){ max.y = _bottom; }
    void setWidth( int _width ){ max.x = min.x + _width; }
    void setHeight( int _height ){ max.y = min.y + _height; }
	int left() const{ return min.x; }
	int top() const{ return min.y; }
	int right() const{ return max.x; }
	int bottom() const{ return max.y; }
	int width() const{ return max.x - min.x; }
	int height() const{ return max.y - min.y; }

    bool pointInside(Vect2i _point) const
    {
      if ( _point.x < min.x || _point.x > max.x )
        return false;
      if ( _point.y < min.y || _point.y > max.y )
        return false;
      return true;
    }

    void serialize(Archive& ar);
};
