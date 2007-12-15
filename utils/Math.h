#pragma once

class Archive;
struct Vector2i{
public:
    int x, y;
    Vector2i() {}
    Vector2i(int _x, int _y)
    : x(_x), y(_y) {}
    void set(int _x, int _y){
        x = _x; y = _y;
    }

    void serialize(Archive& ar);
};
