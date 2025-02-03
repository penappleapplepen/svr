#ifndef SVR_VARIANT_OVERLOADS
#define SVR_VARIANT_OVERLOADS

namespace svr
{
    template <typename... Ts>
    struct overloads : Ts...
    {
        using Ts::operator()...;
    };
}

#endif

#include <iostream>

void test()
{

    const auto visitor = svr::overloads{
        [](int i)
        {
            std::cout << "int\n";
        },
        [](double d)
        {
            std::cout << "double\n";
        }};
    visitor(5);
    visitor(5.5);
}