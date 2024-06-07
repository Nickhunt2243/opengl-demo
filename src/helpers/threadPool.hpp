#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

/// A common thread pool implementation using stop boolean and a mutex I got from chatGPT. Credit where credit is due :)

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <type_traits>

class ThreadPool {
public:
    ThreadPool(size_t);
    ~ThreadPool();
    /**
     *
     * @tparam F:    The type of the enqueued function.
     * @tparam Args: The types of the function args.
     * @param f:     The reference to a function.
     * @param args:  The arguments to pass to the function call.
     * @return:      The invoke_result_t of the pooled function call.
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

inline ThreadPool::ThreadPool(size_t threads)
        : stop(false)
{
    for(size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back(
            [this] {
                for (;;) {
                    std::function < void() > task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                                             [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::invoke_result_t<F, Args...>>
{
    using return_type = typename std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        if(stop)
        {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
    {
        worker.join();
    }
}

#endif // THREADPOOL_HPP
