#ifndef SVR_FIXED_THREAD_POOL
#define SVR_FIXED_THREAD_POOL

#include <bits/stdc++.h>

namespace svr
{
    class FixedThreadPool
    {
    public:
        using Item = std::function<void()>;

    private:
        std::queue<Item> d_queue;
        std::mutex d_mx;
        std::condition_variable d_cv;
        
        std::vector<std::thread> d_threads;

        std::atomic<bool> d_stop{false};
    public:
        FixedThreadPool(int numThreads)
        {
            while (numThreads--)
            {
                d_threads.emplace_back([this]()
                {
                    while(!d_stop.load())
                    {
                        std::vector<Item> items;
                        {
                            std::unique_lock<std::mutex> lk(d_mx);
                            d_cv.wait(lk, [this](){
                                return !d_queue.empty() || d_stop.load();
                            });
                            if(d_stop.load())
                            {
                                break;
                            }
                            items.emplace_back(std::move(d_queue.front()));
                            d_queue.pop();
                        }
                        for(auto& item : items)
                        {
                            item();
                        }
                    } });
            }
        }

        ~FixedThreadPool()
        {
            for (auto &thread : d_threads)
            {
                thread.join();
            }
        }

        void enqueJob(Item item)
        {
            {
                std::lock_guard<std::mutex> lk(d_mx);
                d_queue.push(std::move(item));
            }
            d_cv.notify_one();
        }

        void stop()
        {
            {
                std::lock_guard<std::mutex> lk(d_mx);
                d_stop.store(true);
            }
            d_cv.notify_all();
        }
    };
}

#include <mutex>

void test()
{
    using namespace svr;
    static std::mutex print_mx;

    std::cout << "Test 1: Basic job execution\n";
    {
        FixedThreadPool pool(3);
        std::atomic<int> counter{0};
        for (int i = 0; i < 10; ++i) {
            pool.enqueJob([&counter, i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                ++counter;
                {
                    std::lock_guard<std::mutex> lk(print_mx);
                    std::cout << "Job " << i << " done\n";
                }
            });
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        pool.stop();
        {
            std::lock_guard<std::mutex> lk(print_mx);
            std::cout << "Counter: " << counter.load() << "\n";
        }
    }

    std::cout << "\nTest 2: Stop before all jobs are processed\n";
    {
        FixedThreadPool pool(2);
        std::atomic<int> counter{0};
        for (int i = 0; i < 5; ++i) {
            pool.enqueJob([&counter]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                ++counter;
            });
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        pool.stop();
        {
            std::lock_guard<std::mutex> lk(print_mx);
            std::cout << "Counter (may be < 5): " << counter.load() << "\n";
        }
    }

    std::cout << "\nTest 3: Enqueue after stop\n";
    {
        FixedThreadPool pool(2);
        pool.stop();
        try {
            pool.enqueJob([]() {
                std::lock_guard<std::mutex> lk(print_mx);
                std::cout << "Should not run\n";
            });
            std::lock_guard<std::mutex> lk(print_mx);
            std::cout << "No exception thrown on enqueue after stop\n";
        } catch (...) {
            std::lock_guard<std::mutex> lk(print_mx);
            std::cout << "Exception thrown on enqueue after stop\n";
        }
    }

    std::cout << "\nAll FixedThreadPool tests done.\n";
}

#endif