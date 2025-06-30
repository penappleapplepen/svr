#ifndef SVR_IGNORE
#define SVR_IGNORE

namespace svr
{
    struct _ignore
    {
        template<typename T>
        _ignore& operator=(T&& val)
        {
            return *this;
        }
    };
    _ignore ignore;
}

#endif