#ifndef SVR_VALUE_CATEGORIES
#define SVR_VALUE_CATEGORIES

#include "integral_constant.h"

namespace svr
{
    /**
     * lvalue, prvalue, xvalue is defined for
     * expressions, not values
     * https://stackoverflow.com/a/16638081
     *
     * lvalue may be bound to lvalue reference
     * prvalues and xvalues bind to rvalue reference
     * prvalue and xvalue combined called r value
     * can be bound to const l value reference
     *
     * lvalue reference, r value reference are for types
     *
     * Expression binds to a value
     * Exception for return value in absence of RVO is considered rvalue
     */
    template <typename T>
    struct is_prvalue : svr::true_type
    {
    };
    template <typename T>
    struct is_prvalue<T &> : svr::false_type
    {
    };
    template <typename T>
    struct is_prvalue<T &&> : svr::false_type
    {
    };

    template <typename T>
    struct is_lvalue : svr::false_type
    {
    };
    template <typename T>
    struct is_lvalue<T &> : svr::true_type
    {
    };
    template <typename T>
    struct is_lvalue<T &&> : svr::false_type
    {
    };

    template <typename T>
    struct is_xvalue : svr::false_type
    {
    };
    template <typename T>
    struct is_xvalue<T &> : svr::false_type
    {
    };
    template <typename T>
    struct is_xvalue<T &&> : svr::true_type
    {
    };

    // Alternative implementation
    template <typename T>
    struct value_category
    {
        // Or can be an integral or enum value
        static constexpr auto value = "prvalue";
    };

    template <typename T>
    struct value_category<T &>
    {
        static constexpr auto value = "lvalue";
    };

    template <typename T>
    struct value_category<T &&>
    {
        static constexpr auto value = "xvalue";
    };

    // Double parens for ensuring we inspect an expression,
    // not an entity
    #define VALUE_CATEGORY(expr) value_category<decltype((expr))>::value
}

#endif