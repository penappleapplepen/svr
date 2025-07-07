#include "multithreading/spsc/spscbounded.h"
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <atomic>

#include "multithreading/spsc/rigtorp.h"

using namespace svr;

template <typename QueueType>
void benchmark_spsc(const std::string& name, int num_items) {
    QueueType q;
    std::vector<int> results;
    results.reserve(num_items);
    auto start = std::chrono::high_resolution_clock::now();
    std::thread producer([&]() {
        for (int i = 0; i < num_items; ++i) {
            while (!q.try_push(i)) {}
        }
    });
    std::thread consumer([&]() {
        int val;
        int count = 0;
        while (count < num_items) {
            if (q.try_pop(val)) {
                results.push_back(val);
                ++count;
            }
        }
    });
    producer.join();
    consumer.join();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    double ops_per_ms = num_items / elapsed.count();
    std::cout << name << ": " << num_items << " items, time = " << elapsed.count() << " ms, ops/ms = ";
    if (ops_per_ms >= 1e6) {
        std::cout << (ops_per_ms / 1e6) << " million";
    } else if (ops_per_ms >= 1e3) {
        std::cout << (ops_per_ms / 1e3) << " thousand";
    } else {
        std::cout << ops_per_ms;
    }
    std::cout << std::endl;
    // Optionally check correctness
    bool ok = true;
    for (int i = 0; i < num_items; ++i) {
        if (results[i] != i) { ok = false; break; }
    }
    std::cout << "Correct: " << (ok ? "yes" : "no") << std::endl;
}

// Benchmark for rigtorp::SPSCQueue
template <typename T, size_t N>
void benchmark_spsc_rigtorp(const std::string& name, int num_items) {
    rigtorp::SPSCQueue<T> q(N);
    std::vector<int> results;
    results.reserve(num_items);
    auto start = std::chrono::high_resolution_clock::now();
    std::thread producer([&]() {
        for (int i = 0; i < num_items; ++i) {
            while (!q.try_push(i)) {}
        }
    });
    std::thread consumer([&]() {
        int count = 0;
        while (count < num_items) {
            auto* front = q.front();
            if (front) {
                results.push_back(*front);
                q.pop();
                ++count;
            }
        }
    });
    producer.join();
    consumer.join();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    double ops_per_ms = num_items / elapsed.count();
    std::cout << name << ": " << num_items << " items, time = " << elapsed.count() << " ms, ops/ms = ";
    if (ops_per_ms >= 1e6) {
        std::cout << (ops_per_ms / 1e6) << " million";
    } else if (ops_per_ms >= 1e3) {
        std::cout << (ops_per_ms / 1e3) << " thousand";
    } else {
        std::cout << ops_per_ms;
    }
    std::cout << std::endl;
    // Optionally check correctness
    bool ok = true;
    for (int i = 0; i < num_items; ++i) {
        if (results[i] != i) { ok = false; break; }
    }
    std::cout << "Correct: " << (ok ? "yes" : "no") << std::endl;
}

int main() {
    constexpr int N = 1024*1024;
    constexpr int num_items = 10000000;
    std::cout << "Benchmarking SPSC Queues: " << num_items << " items\n";
    benchmark_spsc<SpscBounded<int, N>>("SpscBounded (lock-free)", num_items);
    benchmark_spsc<SpscBoundedMutex<int, N>>("SpscBoundedMutex (mutex)", num_items);
    benchmark_spsc_rigtorp<int, N>("rigtorp::SPSCQueue", num_items);
    return 0;
}
