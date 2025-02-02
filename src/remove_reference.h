#ifndef SVR_REMOVE_REFERENCE
#define SVR_REMOVE_REFERENCE

namespace svr
{
    template<typename T>
    struct remove_reference
    {
        typedef T type;
    };

    template<typename T>
    struct remove_reference<T&>
    {
        typedef T type;
    };

    template<typename T>
    struct remove_reference<T&&>
    {
        typedef T type;
    };
}

#endif
