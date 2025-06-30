#include "multithreading/spin_lock.h"
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

using namespace svr;

// High contention: all threads increment a shared counter
template <typename LockFunc>
void benchmark(const char* name, LockFunc&& lock_func, int numThreads) {
    int counter = 0;
    int incrementsPerThread = 100000;
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < incrementsPerThread; ++j) {
                lock_func([&](){ ++counter; });
            }
        });
    }
    for (auto& t : threads) t.join();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << name << " (threads=" << numThreads << "): " << elapsed.count() << " ms, counter = " << counter << std::endl;
}

int main() {
    std::cout << "Benchmarking SpinLock vs std::mutex (varying thread count, high contention)\n";
    std::vector<int> thread_counts = {1, 2, 4, 8, 16, 32};
    for (int numThreads : thread_counts) {
        // No lock
        benchmark("No Lock", [](auto&& f){ f(); }, numThreads);
        // SpinLock
        SpinLock spinLock;
        benchmark("SpinLock", [&](auto&& f){ spinLock.lock(); f(); spinLock.unlock(); }, numThreads);
        // std::mutex
        std::mutex mtx;
        benchmark("std::mutex", [&](auto&& f){ std::lock_guard<std::mutex> lk(mtx); f(); }, numThreads);
        std::cout << std::endl;
    }
    return 0;
}
