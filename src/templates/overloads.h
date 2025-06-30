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