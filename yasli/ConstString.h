#pragma once
#include <string.h>

namespace yasli{

class ConstString{
public:
    ConstString(const char* str = "")
    : str_(str) {}
    operator const char*() const{ return str_; }
    bool operator==(const char* rhs) const{
        if(str_ == rhs)
            return true;
        return strcmp(str_, rhs) == 0;
    }
    bool operator!=(const char* rhs) const{
        if(str_ == rhs)
            return false;
        return strcmp(str_, rhs) != 0;
    }
    bool operator<(const char* rhs) const{
        return strcmp(str_, rhs) < 0;
    }
    bool operator>(const char* rhs) const{
        return strcmp(str_, rhs) > 0;
    }
    const char* c_str() const{ return str_; }
protected:
    const char* str_;
};

}
