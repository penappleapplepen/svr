#ifndef SVR_TUPLE
#define SVR_TUPLE

namespace svr
{
    template <typename T, typename... Ts>
    struct tuple
    {
        tuple(const T &t, const Ts &...ts) : value(t), rest(ts...)
        {
        }

        T value;
        tuple<Ts...> rest;
    };

    template <typename T>
    struct tuple<T>
    {
        tuple(const T &t) : value(t)
        {
        }

        T value;
    };

    template <size_t N, typename T, typename... Ts>
    struct nth_type : nth_type<N - 1, Ts...>
    {
        static_assert(N < sizeof...(Ts) + 1,
                      "index out of bounds");
    };

    template <typename T, typename... Ts>
    struct nth_type<0, T, Ts...>
    {
        using value_type = T;
    };

    template <size_t N>
    struct getter
    {
        template <typename T, typename... Ts>
        static nth_type<N, T, Ts...>::value_type &get(tuple<T, Ts...> &t)
        {
            return getter<N - 1>::get(t.rest);
        }
    };

    template <>
    struct getter<0>
    {
        template <typename T, typename... Ts>
        static nth_type<0, T, Ts...>::value_type &get(tuple<T, Ts...> &t)
        {
            return t.value;
        }
    };

    template <size_t N, typename... Ts>
    typename nth_type<N, Ts...>::value_type &get(tuple<Ts...> &ts)
    {
        return getter<N>::get(ts);
    }
}

#endif