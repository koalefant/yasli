#pragma once

#include <vector>

template<class Type>
class Context{
public:
    Context(){
        ASSERT(!stack_.empty());
    }
    Type& operator()(){
		ASSERT(!stack_.empty());
        return *stack_.back();
    }
    Type* operator->(){
		ASSERT(!stack_.empty());
        return *stack_.back();
    }
	static void push(Type* context){
		stack_.push_back(context);
	}
	static void pop(){
		ASSERT(!stack_.empty());
		stack_.pop_back();
	}
protected:
	typedef std::vector<Type*> Stack;
    static Stack stack_;
};

#define IMPLEMENT_CONTEXT(Type) template<class Type> typename Context<Type>::Stack Context<Type>::stack_;
