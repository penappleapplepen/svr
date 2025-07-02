#include "multithreading/spinlock/spinlock.h"
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <string>
#include <mutex>

using namespace svr;

template <typename SpinLockType>
void benchmark_spinlock(const std::string& name, int numThreads, int numIterations) {
    SpinLockType lock;
    std::atomic<int> ready = 0;
    std::atomic<int> finished{0};
    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            // Wait for all threads to be ready
            ++ready;
            while (ready < numThreads) std::this_thread::yield();
            for (int j = 0; j < numIterations; ++j) {
                // CPU work before lock
                volatile int dummy1 = 0;
                for (int k = 0; k < 50; ++k) dummy1 += k * j;

                lock.lock();

                // CPU work inside lock
                volatile int dummy2 = 0;
                for (int k = 0; k < 10; ++k) dummy2 += k + j;

                lock.unlock();

                // CPU work after unlock
                volatile int dummy3 = 0;
                for (int k = 0; k < 50; ++k) dummy3 += k - j;
            }
            ++finished;
        });
    }
    for (auto& t : threads) t.join();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << name << ": threads=" << numThreads << ", time=" << elapsed.count() << " ms" << std::endl;
}

int main() {
    unsigned int max_threads = 2 * std::thread::hardware_concurrency();
    if (max_threads == 0) max_threads = 32; // fallback if detection fails
    std::vector<int> thread_counts;
    for (unsigned int i = 1; i <= max_threads; ++i) thread_counts.push_back(i);
    int numIterations = 1000000;
    std::cout << "Benchmarking SpinLock variants: lock/unlock latency\n";
    std::cout << "Detected hardware threads: " << max_threads << "\n";
    std::cout << "threads,time_ms,variant\n"; // CSV header for plotting
    for (int numThreads : thread_counts) {
        std::cout << "-----------------------------------------------------\n";
        benchmark_spinlock<BasicSpinLockWithAtomicFlag>("BasicSpinLockWithAtomicFlag", numThreads, numIterations);
        benchmark_spinlock<BasicSpinLockWithAtomicBool>("BasicSpinLockWithAtomicBool", numThreads, numIterations);
        benchmark_spinlock<SpinLockWithAtomicFlagWithoutFalseSharing>("SpinLockWithAtomicFlagWithoutFalseSharing", numThreads, numIterations);
        benchmark_spinlock<SpinLockWithAtomicBoolWithoutFalseSharing>("SpinLockWithAtomicBoolWithoutFalseSharing", numThreads, numIterations);
        benchmark_spinlock<SpinLockWithOptimizedLoadsAndThreadYielding>("SpinLockWithOptimizedLoadsAndThreadYielding", numThreads, numIterations);
        benchmark_spinlock<SpinLockWithOptimizedWritesAndThreadYielding>("SpinLockWithOptimizedWritesAndThreadYielding", numThreads, numIterations);
        benchmark_spinlock<std::mutex>("std::mutex", numThreads, numIterations);
        std::cout << "-----------------------------------------------------\n";
    }
    return 0;
}
