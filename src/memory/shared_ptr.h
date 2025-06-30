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

#include <thread>
#include <chrono>
#include <iostream>

// void test()
// {
//     std::cout << "Test 1: Basic creation and destruction\n";
//     {
//         svr::shared_ptr<svr::Test> p1{new svr::Test()};
//         std::cout << "use_count: " << p1.use_count() << "\n";
//     } // Destructor should be called here

//     std::cout << "\nTest 2: Copying shared_ptr\n";
//     {
//         svr::shared_ptr<svr::Test> p2{new svr::Test()};
//         std::cout << "use_count before copy: " << p2.use_count() << "\n";
//         {
//             svr::shared_ptr<svr::Test> p3 = p2;
//             std::cout << "use_count after copy: " << p2.use_count() << "\n";
//         } // Destructor should NOT be called yet
//         std::cout << "use_count after inner scope: " << p2.use_count() << "\n";
//     } // Destructor should be called here

//     std::cout << "\nTest 3: Assignment\n";
//     {
//         svr::shared_ptr<svr::Test> p4{new svr::Test()};
//         svr::shared_ptr<svr::Test> p5{new svr::Test()};
//         std::cout << "p4 use_count: " << p4.use_count() << ", p5 use_count: " << p5.use_count() << "\n";
//         p5 = p4; // Old object should be deleted, ref count updated
//         std::cout << "After assignment, p4 use_count: " << p4.use_count() << ", p5 use_count: " << p5.use_count() << "\n";
//     } // Destructor should be called here

//     std::cout << "\nTest 4: Move semantics\n";
//     {
//         svr::shared_ptr<svr::Test> p6{new svr::Test()};
//         svr::shared_ptr<svr::Test> p7 = std::move(p6);
//         std::cout << "After move, p6 use_count: " << p6.use_count() << ", p7 use_count: " << p7.use_count() << "\n";
//     }

//     std::cout << "\nTest 5: Thread safety\n";
//     {
//         // 1 object
//         svr::shared_ptr<svr::Test> p8{new svr::Test()};
//         // 1 more object inside thread_func
//         auto thread_func = [p8]() mutable
//         {
//             svr::shared_ptr<svr::Test> local = p8;
//             std::cout << "Thread use_count: " << local.use_count() << "\n";
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         };
//         // copy of thread_func is created inside both, so 2 more
//         // then assigned to local, so 2 more
//         std::thread t1(thread_func);
//         std::thread t2(thread_func);
//         t1.join();
//         t2.join();
//         std::cout << "Main thread use_count after threads: " << p8.use_count() << "\n";
//     } // Destructor should be called here

//     std::cout << "\nTest 6: Thread safety\n";
//     {
//         // 1 object
//         svr::shared_ptr<svr::Test> p8{new svr::Test()};
//         // object created in-place, so 1 then assigned to local, so 1 more
//         std::thread t1([p8]() mutable
//                        {
//             svr::shared_ptr<svr::Test> local = p8;
//             std::cout << "Thread use_count: " << local.use_count() << "\n";
//             std::this_thread::sleep_for(std::chrono::milliseconds(10)); });
//         // object created in-place, so 1 then assigned to local, so 1 more
//         std::thread t2([p8]() mutable
//                        {
//             svr::shared_ptr<svr::Test> local = p8;
//             std::cout << "Thread use_count: " << local.use_count() << "\n";
//             std::this_thread::sleep_for(std::chrono::milliseconds(10)); });
//         t1.join();
//         t2.join();
//         std::cout << "Main thread use_count after threads: " << p8.use_count() << "\n";
//     } // Destructor should be called here

//     std::cout << "\nTest 7: Null shared_ptr\n";
//     {
//         svr::shared_ptr<svr::Test> p9;
//         std::cout << "Null shared_ptr use_count: " << p9.use_count() << "\n";
//         if (!p9.get())
//             std::cout << "Null shared_ptr get() returns nullptr\n";
//     }

//     std::cout << "\nAll tests done.\n";
// }

#endif