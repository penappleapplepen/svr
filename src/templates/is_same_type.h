#ifndef SVR_IS_SAME_TYPE
#define SVR_IS_SAME_TYPE

namespace svr
{
    template<typename TYPE1, typename TYPE2, typename... TYPES>
    struct is_same
    {
        static constexpr bool value = is_same<TYPE1,TYPE2>::value && is_same<TYPE2, TYPES...>::value;
    };

    template<typename TYPE>
    struct is_same<TYPE, TYPE>
    {
        static constexpr bool value = true; 
    };

    template<typename TYPE1, typename TYPE2>
    struct is_same<TYPE1, TYPE2>
    {
        static constexpr bool value = false; 
    };

    template<typename... Ts>
    inline constexpr bool is_same_v = is_same<Ts...>::value;
};

#endif