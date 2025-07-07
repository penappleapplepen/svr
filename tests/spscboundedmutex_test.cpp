
#include "multithreading/spsc/spscbounded.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>

using namespace svr;


template <typename Queue>
void BasicPushPopTest() {
    Queue q;
    int val = 0;
    ASSERT_TRUE(q.try_push(42));
    ASSERT_TRUE(q.try_pop(val));
    ASSERT_EQ(val, 42);
}

template <typename Queue>
void FillAndEmptyTest() {
    constexpr size_t N = 8;
    Queue q;
    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(q.try_push(i));
    }
    ASSERT_FALSE(q.try_push(100));
    int val = 0;
    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(q.try_pop(val));
        ASSERT_EQ(val, i);
    }
    ASSERT_FALSE(q.try_pop(val));
}

template <typename Queue>
void WrapAroundTest() {
    constexpr size_t N = 4;
    Queue q;
    int val = 0;
    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(q.try_push(i));
    }
    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(q.try_pop(val));
        ASSERT_EQ(val, i);
    }
    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(q.try_push(i + 10));
    }
    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(q.try_pop(val));
        ASSERT_EQ(val, i + 10);
    }
}

template <typename Queue>
void SpscThreadedTest() {
    constexpr size_t N = 64;
    constexpr int total = 10000;
    Queue q;
    std::atomic<bool> done{false};
    std::vector<int> results;
    std::thread producer([&]() {
        for (int i = 0; i < total; ++i) {
            while (!q.try_push(i)) {}
        }
        done = true;
    });
    std::thread consumer([&]() {
        int val;
        int count = 0;
        while (!done || count < total) {
            if (q.try_pop(val)) {
                results.push_back(val);
                ++count;
            }
        }
    });
    producer.join();
    consumer.join();
    ASSERT_EQ(results.size(), total);
    for (int i = 0; i < total; ++i) {
        ASSERT_EQ(results[i], i);
    }
}

// Instantiate tests for both SpscBounded and SpscBoundedMutex
TEST(SpscBoundedTest, BasicPushPop) { BasicPushPopTest<SpscBounded<int, 8>>(); }
TEST(SpscBoundedTest, FillAndEmpty) { FillAndEmptyTest<SpscBounded<int, 8>>(); }
TEST(SpscBoundedTest, WrapAround) { WrapAroundTest<SpscBounded<int, 4>>(); }
TEST(SpscBoundedTest, SpscThreaded) { SpscThreadedTest<SpscBounded<int, 64>>(); }

TEST(SpscBoundedMutexTest, BasicPushPop) { BasicPushPopTest<SpscBoundedMutex<int, 8>>(); }
TEST(SpscBoundedMutexTest, FillAndEmpty) { FillAndEmptyTest<SpscBoundedMutex<int, 8>>(); }
TEST(SpscBoundedMutexTest, WrapAround) { WrapAroundTest<SpscBoundedMutex<int, 4>>(); }
TEST(SpscBoundedMutexTest, SpscThreaded) { SpscThreadedTest<SpscBoundedMutex<int, 64>>(); }
