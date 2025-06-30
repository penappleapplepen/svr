#ifndef SVR_FORWARD
#define SVR_FOWRARD

#include "remove_reference.h"

/**
    Caller may pass lvalue, prvalue, xvalue since it is an expression
    Callee may bind it to value, (possibly const) lvalue reference, (possibly const) rvalue reference

    https://stackoverflow.com/a/46560302
    https://stackoverflow.com/a/46560485
    https://stackoverflow.com/a/47125430

    My explanation after running it through cppinsights:
    1) When you have template<typename T> void foo(T&& arg). Basically a function taking in universal
    reference. 
    a) When you call foo(a), l-value makes typename T as T&, so T& && collapses to T& which is both
    T and type of arg
    b) When you call foo(5), foo(std::move(a)), r-value makes typename T as T, so T && becaomes T&& 
    Here, T is T and arg is of type T&&

    We explicitly mention type for remove_reference, so we need to specialize on types

    Now, coming to how forward works, let's take an example

    template<typename T>
    void foo(T&& value)
    {
        // Case 1: foo(a)
        // T is int&, T&& -> int& && -> int&
        // T and type of value is int&
        // Case 2 and Case 3: foo(5), foo(std::move(a))
        // T is int, T&& -> int&&
        // T is of type int, value is of type int&&
        
        forward<T>(value);

        // In case 1, it will call forward<int&>(value);
        // In case 2 and 3, it will call forward<int>(value);
        // It is important to note that, value is l-value expression
        // so it will always call the overload taking l-value reference
        // In case 1 -> forward resolves to 
        template<>
        int& && forward(typename svr::remove_reference<int&>::type &value)
        {
            return static_cast<int& &&>(value);
        }
        so it returns l-value expression after reference collapsing
        // In case 2 and case 3 -> forward resolves to
        template<>
        int && forward(typename svr::remove_reference<int>::type &value)
        {
            return static_cast<int &&>(value);
        }

        If you notice, both of them called the first overload of forward,
        to call second overload of forward, you would need to do something like
        this
        forward<T>(5);
        which is called from foo(5)
    }

    int main()
    {
        int a = 5;
        foo(a);
        foo(5);
        foo(std::move(a));
    }
*/

namespace svr
{
    // we use remove_reference::type to make it non-deducible
    // context, so whoever calls forward, lets us know T
    // because we can't figure it out from the value they pass
    // which is always an l-value expression
    template<typename T>
    T&& forward(typename svr::remove_reference<T>::type &value)
    {
        return static_cast<T&&>(value);
    }

    template<typename T>
    T&& forward(typename svr::remove_reference<T>::type &&value)
    {
        return static_cast<T&&>(value);
    }
} 

#endif

