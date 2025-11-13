#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief 一个简单的线程池实现
 *
 * 这个线程池实现了以下特性：
 * - 固定大小的线程池
 * - 任务队列管理
 * - 优雅关闭机制
 * - 支持获取任务执行结果
 */
class ThreadPool {
  public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    // 禁止拷贝构造和赋值
    ThreadPool(const ThreadPool &)             = delete;
    ThreadPool & operator=(const ThreadPool &) = delete;

    /**
     * @brief 提交任务到线程池
     * @param task 要执行的任务
     * @return std::future 用于获取任务结果
     */
    template <typename F, typename... Args>
    auto submit(F && f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    /**
     * @brief 获取当前等待执行的任务数量
     * @return size_t 任务数量
     */
    size_t getPendingTaskCount() const;

    /**
     * @brief 关闭线程池
     */
    void shutdown();

  private:
    std::vector<std::thread>          workers_;
    std::queue<std::function<void()>> tasks_;

    mutable std::mutex      queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool>       stop_;
};

// 模板方法实现
template <typename F, typename... Args>
auto ThreadPool::submit(F && f,
                        Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> result = task->get_future();
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("Cannot submit task to stopped thread pool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return result;
}

#endif // THREAD_POOL_H
