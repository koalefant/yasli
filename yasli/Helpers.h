/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

namespace yasli{

class Archive;
namespace Helpers{

template<bool C, class T1, class T2>
struct Selector{};

template<class T1, class T2>
struct Selector<false, T1, T2>{
    typedef T2 type;
};

template<class T1, class T2>
struct Selector<true, T1, T2>{
    typedef T1 type;
};

template<class C, class T1, class T2>
struct Select{
    typedef typename Selector<C::value, T1,T2>::type selected_type;
    typedef typename selected_type::type type;
};

template<class T>
struct Identity{
    typedef T type;
};


template<class T>
struct IsArray{
    enum{ value = false };
};

template<class T, int Size>
struct IsArray< T[Size] >{
    enum{ value = true };
};

template<class T, int Size>
struct ArraySize{
    enum{ value = true };
};

template<class T>
struct IsClass{
private:
    struct NoType{ char dummy; };
    struct YesType{ char dummy[100]; };

    template<class U>
    static YesType function_helper(void(U::*)(void));

    template<class U>
    static NoType function_helper(...);
public:
    enum{ value = (sizeof(function_helper<T>(0)) == sizeof(YesType))};
};

}
}

