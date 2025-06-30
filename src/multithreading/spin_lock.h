#ifndef SVR_SPIN_LOCK
#define SVR_SPIN_LOCK

#include <atomic>

namespace svr
{
    class SpinLock
    {
        private:
            std::atomic<bool> isLocked{false};
        public:
            void lock()
            {
                while(isLocked.exchange(true, std::memory_order_acquire));
            }

            void unlock()
            {
                isLocked.store(false, std::memory_order_release);
            }
    }; 
}

#endif