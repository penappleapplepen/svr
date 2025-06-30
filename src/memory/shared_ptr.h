#ifndef SVR_SHARED_PTR
#define SVR_SHARED_PTR

#include "atomic"
#include "test.h"

namespace svr
{
    template <typename TYPE>
    class shared_ptr;

    template <typename TYPE>
    class control_block
    {
    private:
        TYPE *d_instance;
        std::atomic<long> d_refCnt;

    public:
        friend class shared_ptr<TYPE>;

        control_block(TYPE *instance) : d_instance(instance), d_refCnt(1)
        {
        }

        ~control_block()
        {
            delete d_instance;
        }
    };

    template <typename TYPE>
    class shared_ptr
    {
    private:
        svr::control_block<TYPE> *d_cntrl;

    public:
        shared_ptr() : d_cntrl(nullptr) {}
        explicit shared_ptr(TYPE *instance) : d_cntrl(new control_block(instance)) {}
        shared_ptr(const shared_ptr &other) : d_cntrl(other.d_cntrl)
        {
            if (d_cntrl)
            {
                d_cntrl->d_refCnt.fetch_add(1, std::memory_order_relaxed);
            }
        }
        shared_ptr(shared_ptr &&other) : d_cntrl(other.d_cntrl)
        {
            other.d_cntrl = nullptr;
        }
        shared_ptr &operator=(const shared_ptr &other)
        {
            if (this == &other)
            {
                return *this;
            }

            if (d_cntrl && d_cntrl->d_refCnt.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete d_cntrl;
            }
            d_cntrl = other.d_cntrl;
            if (d_cntrl)
            {
                d_cntrl->d_refCnt.fetch_add(1, std::memory_order_relaxed);
            }

            return *this;
        }
        shared_ptr &operator=(shared_ptr &&other)
        {
            if (this == &other)
            {
                return *this;
            }
            if (d_cntrl && d_cntrl->d_refCnt.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete d_cntrl;
            }
            d_cntrl = other.d_cntrl;
            other.d_cntrl = nullptr;
            return *this;
        }
        ~shared_ptr()
        {
            if (d_cntrl && d_cntrl->d_refCnt.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete d_cntrl;
            }
        }
        TYPE *get() const
        {
            return d_cntrl ? d_cntrl->d_instance : nullptr;
        }

        TYPE *operator->() const
        {
            return d_cntrl->d_instance;
        }

        TYPE &operator*() const
        {
            return *(d_cntrl->d_instance);
        }

        long use_count() const
        {
            return d_cntrl ? d_cntrl->d_refCnt.load() : 0;
        }
    };
}

#endif