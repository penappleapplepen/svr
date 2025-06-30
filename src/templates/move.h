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