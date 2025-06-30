#include <gtest/gtest.h>
#include "../src/multithreading/fixed_thread_pool.h"
#include <atomic>
#include <vector>
#include <thread>
#include <chrono>

using namespace svr;

TEST(FixedThreadPoolTest, SingleJobExecutes) {
    FixedThreadPool pool(2);
    std::atomic<bool> flag{false};
    pool.enqueJob([&flag]{ flag = true; });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(flag.load());
    pool.stop();
}

TEST(FixedThreadPoolTest, MultipleJobsExecute) {
    FixedThreadPool pool(4);
    std::atomic<int> counter{0};
    int numJobs = 10;
    for (int i = 0; i < numJobs; ++i) {
        pool.enqueJob([&counter]{ counter.fetch_add(1); });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter.load(), numJobs);
    pool.stop();
}

TEST(FixedThreadPoolTest, StopStopsThreads) {
    FixedThreadPool pool(2);
    std::atomic<int> counter{0};
    pool.enqueJob([&counter]{ std::this_thread::sleep_for(std::chrono::milliseconds(50)); counter++; });
    pool.stop();
    int afterStop = counter.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(counter.load(), afterStop); // No new jobs should run after stop
}

TEST(FixedThreadPoolTest, EnqueueAfterStopDoesNotRun) {
    FixedThreadPool pool(2);
    pool.stop();
    std::atomic<bool> flag{false};
    pool.enqueJob([&flag]{ flag = true; });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(flag.load());
}
