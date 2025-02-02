#ifndef SVR_INTEGRAL_CONSTANT
#define SVR_INTEGRAL_CONSTANT

namespace svr
{
    template <typename T, T v>
    struct integral_constant
    {
        static constexpr T value = v;
    };

    // Specialization
    using true_type = svr::integral_constant<bool, true>;
    using false_type = svr::integral_constant<bool, false>;
}

#endif