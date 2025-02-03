#ifndef SVR_REMOVE_REFERENCE
#define SVR_REMOVE_REFERENCE

namespace svr
{
    template <typename T>
    struct remove_reference
    {
        typedef T type;
    };

    template <typename T>
    struct remove_reference<T &>
    {
        typedef T type;
    };

    template <typename T>
    struct remove_reference<T &&>
    {
        typedef T type;
    };

    template<typename T>
    using remove_reference_t = typename remove_reference<T>::type;
}

#endif
