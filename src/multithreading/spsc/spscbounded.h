#ifndef SVR_SPSC_BOUNDED
#define SVR_SPSC_BOUNDED

#include <atomic>
#include <memory>

namespace svr
{   /**
    This queue has size limit of max size_t push and pop. Rigtorp's can have more at the 
    cost of having one empty slot
    */
    template<typename T, size_t N, typename Alloc=std::allocator<T>>
    class SpscBounded
    {
        #if defined(__cpp_lib_hardware_interference_size)
        #define SVR_CACHELINE_SIZE std::hardware_destructive_interference_size
        #else
        #define SVR_CACHELINE_SIZE 64
        #endif

        static_assert(std::atomic<size_t>::is_always_lock_free);
        static_assert(N != 0, "Size of queue cannot be 0");
        static_assert((N & (N-1)) == 0, "Size of queue must be a multiple of 2");
        // We need to make sure there are no false sharing for slots array as well
        // If we assume a cache line is 64 bytes, we need to make sure that irrespective
        // of whether this slots array fits at start/middle/end of cache line, there won't
        // be false sharing. This means we will need to allocate extra space before and after slots
        // Let's assume worst case scenario of just 1 element in array and this is a bool. Then
        // if this is at start of cache line, we have 63 bytes after, or 63 bytes before, so we
        // allocate 2 * 63 bytes before and after. We can be quite lenient with it and allocate
        // enough dummy elements to fit in cache line before and after
        static constexpr size_t NUM_PADDING_ELEMENTS = (SVR_CACHELINE_SIZE - 1) / sizeof(T) + 1;

        private:
            [[no_unique_address]] Alloc d_alloc;
            T* d_arr;

            alignas(SVR_CACHELINE_SIZE) size_t d_cachedTailIndex{0};
            alignas(SVR_CACHELINE_SIZE) size_t d_cachedHeadIndex{0};
            alignas(SVR_CACHELINE_SIZE) std::atomic<size_t> d_tailIndex{0};
            alignas(SVR_CACHELINE_SIZE) std::atomic<size_t> d_headIndex{0};
            char padding_[SVR_CACHELINE_SIZE - sizeof(std::atomic<size_t>)];
        public:
            SpscBounded(const SpscBounded &) = delete;
            SpscBounded(SpscBounded&&) = delete;
            SpscBounded &operator=(const SpscBounded &) = delete;
            SpscBounded &operator=(SpscBounded &&) = delete;

            SpscBounded() : d_arr(d_alloc.allocate(N + 2*NUM_PADDING_ELEMENTS))
            {
                
            }

            ~SpscBounded()
            {
                d_alloc.deallocate(d_arr, N + 2*NUM_PADDING_ELEMENTS);
            }

            template<typename U>
            bool try_push(U&& ele)
            {
                size_t tailIndex = d_tailIndex.load(std::memory_order_relaxed);

                // array is full
                if(tailIndex == d_cachedHeadIndex + N) [[unlikely]]
                {
                    d_cachedHeadIndex = d_headIndex.load(std::memory_order_acquire);
                    if(tailIndex == d_cachedHeadIndex + N) [[unlikely]]
                    {
                        return false;
                    }
                }

                new(&d_arr[(tailIndex & (N-1)) + NUM_PADDING_ELEMENTS]) T(std::forward<U>(ele));
                d_tailIndex.fetch_add(1, std::memory_order_release);

                return true;
            }

            bool try_pop(T& val)
            {
                size_t headIndex = d_headIndex.load(std::memory_order_relaxed);

                // array is empty
                if(headIndex == d_cachedTailIndex) [[unlikely]]
                {
                    d_cachedTailIndex = d_tailIndex.load(std::memory_order_acquire);
                    if(headIndex == d_cachedTailIndex) [[unlikely]]
                    {
                        return false;
                    }
                }

                size_t wrappedIndex = (headIndex & (N-1)) + NUM_PADDING_ELEMENTS;
                val = std::move(d_arr[wrappedIndex]);
                d_arr[wrappedIndex].~T();
                d_headIndex.fetch_add(1, std::memory_order_release);

                return true;
            } 
    };

    template<typename T, size_t N, typename Alloc=std::allocator<T>>
    class SpscBoundedMutex
    {
        static_assert(N != 0, "Size of queue cannot be 0");
        static_assert((N & (N-1)) == 0, "Size of queue must be a multiple of 2");
        private:
            [[no_unique_address]] Alloc d_alloc;
            std::mutex d_mx;
            T* d_arr{nullptr};
            size_t headIndex{0};
            size_t tailIndex{0};
        public:
            SpscBoundedMutex():d_arr(d_alloc.allocate(N))
            {

            }

            ~SpscBoundedMutex()
            {
                d_alloc.deallocate(d_arr, N);
            }

            template<typename U>
            bool try_push(U&& val)
            {
                std::lock_guard<std::mutex> lk(d_mx);
                if(tailIndex == headIndex + N)
                {
                    return false;
                }
                new (&d_arr[tailIndex % N]) T(std::forward<U>(val));
                tailIndex += 1;

                return true;
            }

            bool try_pop(T& val)
            {
                std::lock_guard<std::mutex> lk(d_mx);
                if(tailIndex == headIndex)
                {
                    return false;
                }
                val = std::move(d_arr[headIndex % N]);
                d_arr[headIndex % N].~T();
                headIndex++;

                return true;
            }
    };
}

#endif