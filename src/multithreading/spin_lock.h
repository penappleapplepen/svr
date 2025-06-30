/**
spinlock causes each thread to waste CPU cycles. It can cause thread which acquire the spinlock to get
less cpu cycles because other threads are just busy spinning. mutex puts other threads to sleep
with cores >= threads, spin lock is better, because there is no overhead of OS and it's a direct
hardware operation

We introduce the following optimizations in our spin lock component
1) Prevent false sharing of the atomic variable. If there is some other variable on cache line,
updates to that will also cause cache invalidation, so we can reserve the whole cache line
for this variable
2) In lock() function, we could do while(!atomic.exchange(true));, but this makes every operation
a RMW, so every exchange() will cause cache lines to bounce. If we actually split it into two parts
a) load() b) compare_exchange_strong(), then we can only perform the RMW when load() returns false,
because this signifies that variable is not locked and there is still opportunity for locking it. We 
also use appropriate memory_order. We know that we only need acquire, we compare_exchange_strong succeeds
3) We do a thread yield as soon as we notice that load() returns true, which means that some other thread
acquired this lock. If we have n threads, 1 will acquire it and n-1 will yield, so acquired thread can do it's
operation faster
4) compiler hinting with __builtin_expect helps branch prediction
5) We deliberately don't use the notify_one() and wait() of atomics, because that would not
make it spin lock
*/

#ifndef SVR_SPIN_LOCK
#define SVR_SPIN_LOCK

#include <atomic>
#include <new>
#include <thread>

#if defined(__cpp_lib_hardware_interference_size)
#define SVR_CACHELINE_SIZE std::hardware_destructive_interference_size
#else
#define SVR_CACHELINE_SIZE 64
#endif

namespace svr
{
    #define likely(x) __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)

    class SpinLock
    {
        private:
            // avoid false sharing by aligning to cache line and adding padding
            // so nothing else ends up on the same cache line 
            alignas(SVR_CACHELINE_SIZE) std::atomic<bool> isLocked{false};
            char padding_[SVR_CACHELINE_SIZE - sizeof(std::atomic<bool>)]{};
        public:
            void lock()
            {
                // exchange is a RMW operation which is expensive. Once a thread
                // acquires the mutex lock, we know that other threads just have to 
                // wait. Why do we make them do exchange then? can we make them do a 
                // plain load(). Yes! We do a plain load, if it returns false, then we 
                // can do a CAS
                while(true)
                {
                    if(unlikely(!isLocked.load(std::memory_order_relaxed)))
                    {
                        bool expected = false;
                        isLocked.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed);
                        if(likely(!expected))
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
}

#endif