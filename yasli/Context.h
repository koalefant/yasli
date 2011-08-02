/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>

namespace yasli{

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

}

#define IMPLEMENT_CONTEXT(Type) template<class Type> typename Context<Type>::Stack Context<Type>::stack_;
