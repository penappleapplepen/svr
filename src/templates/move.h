#ifndef SVR_MOVE
#define SVR_MOVE

#include "remove_reference.h"

namespace svr
{
    template <typename T>
    typename svr::remove_reference<T>::type &&move(T &&val)
    {
        return static_cast<typename svr::remove_reference<T>::type &&>(val);
    }
}

#endif

// void test()
// {
//     int val;
//     Custom cust;
//     std::cout << svr::is_rvalue_reference<decltype(svr::move(val))>::value << std::endl;
//     std::cout << svr::is_rvalue_reference<decltype(svr::move(cust))>::value << std::endl;
//     std::cout << svr::is_rvalue_reference<decltype(svr::move(5))>::value << std::endl;
//     std::cout << svr::is_rvalue_reference<decltype(svr::move(Custom{}))>::value << std::endl;
// }