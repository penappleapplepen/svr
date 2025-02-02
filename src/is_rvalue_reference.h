#ifndef SVR_IS_RVALUE_REFERENCE
#define SVR_IS_RVALUE_REFERENCE

#include "integral_constant.h"

namespace svr
{
    template <typename T>
    struct is_rvalue_reference : svr::false_type
    {
    };

    template <typename T>
    struct is_rvalue_reference<T &&> : svr::true_type
    {
    };
}

#endif