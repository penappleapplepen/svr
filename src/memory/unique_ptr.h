#ifndef SVR_UNIQUE_PTR
#define SVR_UNIQUE_PTR

#include <memory>
#include <utility>
#include <type_traits>
#include "templates/forward.h"
#include "helpers/test.h"
#include "templates/move.h"

namespace svr
{
    template <typename TYPE, typename DELETER = std::default_delete<TYPE>>
    class unique_ptr
    {
    private:
        TYPE *d_ptr;
        [[no_unique_address]] DELETER d_deleter;

    private:
        unique_ptr(const unique_ptr &) = delete;
        unique_ptr &operator=(const unique_ptr &) = delete;

    public:
        unique_ptr() : d_ptr(nullptr) {  }
        unique_ptr(TYPE * const ptr) : d_ptr(ptr) {}
        unique_ptr(TYPE * const ptr, DELETER deleter): d_ptr(ptr), d_deleter(deleter) {}
        ~unique_ptr()
        {
            d_deleter(d_ptr);
        }
        unique_ptr(unique_ptr &&other): d_ptr(other.d_ptr), d_deleter(svr::move(other.d_deleter))
        {
            other.d_ptr = nullptr;
        }

        unique_ptr& operator=(unique_ptr&& other)
        {
            if(this != &other)
            {
                delete d_ptr;
                d_ptr = other.d_ptr;
                d_deleter = svr::move(d_deleter);
                other.d_ptr = nullptr;
            }
            
            return *this;
        }

        TYPE *release()
        {
            TYPE *oldPtr = d_ptr;
            d_ptr = nullptr;
            return oldPtr;
        }

        void reset(TYPE *newPtr)
        {
            delete d_ptr;
            d_ptr = newPtr;
        }

        TYPE *get()
        {
            return d_ptr;
        }

        TYPE& operator*()
        {
            return *d_ptr;
        }

        TYPE* operator->()
        {
            return d_ptr;
        }
    };

    template<typename TYPE, typename... Args>
    unique_ptr<TYPE> make_unique(Args&&... args)
    {
        return svr::unique_ptr<TYPE>(new TYPE{svr::forward<Args>(args)...});
    }

    template<typename TYPE, typename DELETER, typename... Args>
    unique_ptr<TYPE> make_unique(DELETER&& deleter, Args&&... args)
    {
        return svr::unique_ptr<TYPE, DELETER>(new TYPE{svr::forward<Args>(args)...}, svr::forward<DELETER>(deleter));
    }
}

#endif