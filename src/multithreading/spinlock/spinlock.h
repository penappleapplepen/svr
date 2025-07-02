/**
spinlock causes each thread to waste CPU cycles. It can cause thread which acquire the spinlock to get
less cpu cycles because other threads are just busy spinning. mutex puts other threads to sleep
with cores >= threads, spin lock is better, because there is no overhead of OS and it's a direct
hardware operation. It is possible that thread holding spin lock gets scheduled out causing it to be slow.
If time spent in acquiring mutex and putting other threads to sleep is more than time spent in critical section,
might as well use spin locks.

On a single core, other threads will go to sleep and let the main thread do work. 
If CPU affinity or pinning is used, thread A and thread B can be producer and consumer and
the cache lines will only have to be transferred between two cores instead of all the available
cores. It might be faster for these threads to spinlock than be put to sleep, especially if you
have dedicated cores. It also provides cache benefits as cache is not removed. Two factors,
cache retention and less cache ping-pong

We introduce the following optimizations in our spin lock component
1) Prevent false sharing of the atomic variable. If there is some other variable on cache line,
updates to that will also cause cache invalidation, so we can reserve the whole cache line
for this variable
2) In lock() function, we could do while(!atomic.exchange(true));, but this makes every operation
a RMW, so every exchange() will cause cache lines to bounce. If we actually split it into two parts
a) load() b) compare_exchange_strong(), then we can only perform the RMW when load() returns false,
because this signifies that variable is not locked and there is still opportunity for locking it. We 
also use appropriate memory_order. We know that we only need acquire, we compare_exchange_strong succeeds.
We can also optimize it for the acquire case
3) We do a thread yield as soon as we notice that load() returns true, which means that some other thread
acquired this lock. If we have n threads, 1 will acquire it and n-1 will yield, so acquired thread can do it's
operation faster
4) compiler hinting with [[likely]] and [[unlikely]] helps branch prediction. We want to make lock acquiring path faster, because
other threads can reach the waiting point slowly, no need to "hurry up" wait, so we need to optimize for acquiring
thread to reach there faster
5) We deliberately don't use the notify_one() and wait() of atomics, because that would not
make it spin lock
6) compare_exchange_weak() in loop performs better on some RISC architectures instead of 
compare_exchange_strong()
7) It might be nice to spin a few times before yielding. This is because spin locks are used for infrequently locked
and small code blocks, so it might be the case that spinning a few more times will get the lock
8) You need to think about throughput. In this case, when a thread releases the lock, there is a high
change that it reacquires it back, so one thread is doing a lot of work, but we need every thread to make progress
as well. Think about usage before thinking about the optimizations
9) _mm_pause() can be used if hyperthreading is enabled
*/

#ifndef SVR_SPIN_LOCK
#define SVR_SPIN_LOCK

#include <atomic>
#include <new>
#include <thread>

namespace svr
{
    class BasicSpinLockWithAtomicFlag
    {
        private:
            std::atomic_flag isLocked{};
        public:
            void lock()
            {
                while(isLocked.test_and_set(std::memory_order_acquire));
            }

            void unlock()
            {
                isLocked.clear(std::memory_order_release);
            }
    };

    class BasicSpinLockWithAtomicBool
    {
        private:
            std::atomic<bool> isLocked{false};
        public:
            void lock()
            {
                while(!isLocked.exchange(true, std::memory_order_acquire));
            }

            void unlock()
            {
                isLocked.store(false, std::memory_order_release);
            }
    };

    #if defined(__cpp_lib_hardware_interference_size)
    #define SVR_CACHELINE_SIZE std::hardware_destructive_interference_size
    #else
    #define SVR_CACHELINE_SIZE 64
    #endif
    class SpinLockWithAtomicFlagWithoutFalseSharing
    {
        private:
            alignas(SVR_CACHELINE_SIZE) std::atomic_flag isLocked{};
            char padding_[SVR_CACHELINE_SIZE - sizeof(std::atomic_flag)];
        public:
            void lock()
            {
                while(isLocked.test_and_set(std::memory_order_acquire));
            }

            void unlock()
            {
                isLocked.clear(std::memory_order_release);
            }
    };
    
    class SpinLockWithAtomicBoolWithoutFalseSharing
    {
        private:
            alignas(SVR_CACHELINE_SIZE) std::atomic<bool> isLocked;
            char padding_[SVR_CACHELINE_SIZE - sizeof(std::atomic<bool>)];
        public:
            void lock()
            {
                while(!isLocked.exchange(true, std::memory_order_acquire));
            }

            void unlock()
            {
                isLocked.store(false, std::memory_order_release);
            }
    };

    class SpinLockWithOptimizedLoadsAndThreadYielding
    {
        private:
            alignas(SVR_CACHELINE_SIZE) std::atomic<bool> isLocked{false};
            char padding_[SVR_CACHELINE_SIZE - sizeof(std::atomic<bool>)]{};
        public:
            void lock()
            {
                // test and test and set
                while(true)
                {
                    if(!isLocked.load(std::memory_order_relaxed)) [[likely]]
                    {
                        bool expected = false;
                        isLocked.compare_exchange_weak(expected, true, std::memory_order_acquire, std::memory_order_relaxed);
                        if(!expected) [[likely]]
                        {
                            break;
                        }
                    }
                    
                    // yield core 
                    std::this_thread::yield();
                }
            }

            void unlock()
            {
                isLocked.store(false, std::memory_order_release);
            }
    };

    class SpinLockWithOptimizedWritesAndThreadYielding
    {
        private:
            alignas(SVR_CACHELINE_SIZE) std::atomic<bool> isLocked{false};
            char padding_[SVR_CACHELINE_SIZE - sizeof(std::atomic<bool>)]{};
        public:
            void lock()
            {
                // test and test and set
                while(true)
                {
                    if(!isLocked.exchange(true, std::memory_order_acquire))
                    {
                        break;
                    }

                    while(isLocked.load(std::memory_order_relaxed))
                    {
                        // yield core 
                        std::this_thread::yield();
                    }
                }
            }

            void unlock()
            {
                isLocked.store(false, std::memory_order_release);
            }
    };
}

#endif