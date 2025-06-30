#ifndef SVR_FIXED_THREAD_POOL
#define SVR_FIXED_THREAD_POOL

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <iostream>
#include <chrono>

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

#endif