#pragma once

template<class Type>
class Context{
public:
    Context(){
        ASSERT(current_);
    }
    Type& operator()(){
        return *current_;
    }
    Type* operator->(){
        return current_;
    }
    static void free(){
        ASSERT(current_ != 0);
        current_ = 0;
    }
    static void set(Type* newContext){
        ASSERT(current_ == 0);
        current_ = newContext;
    }
protected:
    static Type* current_;
};
