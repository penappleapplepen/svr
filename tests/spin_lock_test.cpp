#include <gtest/gtest.h>
#include "multithreading/spin_lock.h"
#include <thread>
#include <vector>

using namespace svr;

TEST(SpinLockTest, SingleThreadLockUnlock) {
    SpinLock lock;
    lock.lock();
    lock.unlock();
    SUCCEED();
}

TEST(SpinLockTest, MultiThreadedIncrement) {
    SpinLock lock;
    int counter = 0;
    const int numThreads = 8;
    const int incrementsPerThread = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < incrementsPerThread; ++j) {
                lock.lock();
                ++counter;
                lock.unlock();
            }
        });
    }
    for (auto& t : threads) t.join();
    EXPECT_EQ(counter, numThreads * incrementsPerThread);
}
