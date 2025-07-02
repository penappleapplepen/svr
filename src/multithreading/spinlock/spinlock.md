# Spin Lock Design and Optimizations

Spin locks are synchronization primitives that cause each thread to waste CPU cycles while waiting for the lock. This can result in the thread that acquires the spin lock receiving fewer CPU cycles, as other threads are also busy spinning in the limited number of cores. In contrast, a mutex puts other threads to sleep, which is more efficient when the critical section is long or contention is high. Spin locks can cause priority inversion where a higher priority thread cannot get the lock for a long time

## When to Use Spin Locks
- **Cores ≥ Threads:** Spin locks are preferable when the number of CPU cores is greater than or equal to the number of threads, as there is no OS overhead and the lock is a direct hardware operation. On a single core, with a mutex, other threads will sleep, allowing the main thread to work
- **Short Critical Sections:** If the time spent acquiring a mutex and putting threads to sleep exceeds the time spent in the critical section, spin locks may be more efficient.
- **CPU Affinity/Pinning:** With CPU affinity or pinning, producer and consumer threads can benefit from cache locality, reducing cache line transfers and improving performance. Spin locks can help retain cache lines and reduce cache ping-pong. We can also use __mm_pause() if power
consumption is an issue in hyperthreads. This is usually the case with HFTs where producer and consumer are on threads T1 and T2, so all cache transfer and retention happens between these two cores only

## Optimizations in Our Spin Lock Component
1. **Prevent False Sharing:** Reserve the entire cache line for the atomic variable to avoid cache invalidation from other variables on the same line.
2. **Test-and-Test-and-Set:** Instead of `while(!atomic.exchange(true))`, which causes every operation to be a read-modify-write (RMW), split into:
   - `load()`
   - `compare_exchange_weak()`
   Only perform the RMW when `load()` returns false, reducing cache line bouncing. Use appropriate memory orderings (acquire on success, relaxed on failure).
3. **Thread Yielding:** Yield the thread as soon as `load()` returns true, allowing the thread that acquired the lock to proceed faster. With `n` threads, one acquires the lock while the others yield, improving throughput.
4. **Branch Prediction Hints:** Use `[[likely]]` and `[[unlikely]]` to optimize the lock acquisition path, as waiting threads can afford to be slower. This might not neccessarily help optimize CPU branch prediction, but could help with instruction cache
5. **No `notify_one()` or `wait()`:** These atomic operations are not used, as they would defeat the purpose of a spin lock.
6. **`compare_exchange_weak()` in Loops:** On some RISC architectures, `compare_exchange_weak()` performs better than `compare_exchange_strong()` in a loop.
7. **Spin Before Yielding:** For infrequently locked and short critical sections, spinning a few times before yielding can improve performance.
8. **Benchmark Considerations:** When a thread releases the lock, it may reacquire it immediately, leading to one thread doing most of the work. Optimizations should ensure all threads make progress.

---

This documentation summarizes the design decisions and optimizations implemented in the spin lock component. For code examples and further details, see the corresponding [header file](spinlock.h).
