#include <chrono>
#include <iostream>
#include <vector>

#include "thread_pool.h"

/**
 * @brief 演示简单的计算任务
 */
void demonstrateSimpleTasks() {
    ThreadPool pool(4); // 创建4个工作线程的线程池

    // 提交一些简单的计算任务
    std::vector<std::future<int>> results;

    for (int i = 0; i < 8; ++i) {
        auto future = pool.submit([i] {
            std::cout << "Task " << i << " is running on thread " << std::this_thread::get_id()
                      << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        });
        results.push_back(std::move(future));
    }

    // 获取并打印结果
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "Task " << i << " result: " << results[i].get() << std::endl;
    }
}

/**
 * @brief 演示长时间运行的任务
 */
void demonstrateLongRunningTasks() {
    ThreadPool pool(2); // 使用2个工作线程

    // 提交一些长时间运行的任务
    auto future1 = pool.submit([] {
        std::cout << "Long task 1 started\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return "Task 1 completed";
    });

    auto future2 = pool.submit([] {
        std::cout << "Long task 2 started\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return "Task 2 completed";
    });

    // 等待并获取结果
    std::cout << future1.get() << std::endl;
    std::cout << future2.get() << std::endl;
}

/**
 * @brief 演示异常处理
 */
void demonstrateExceptionHandling() {
    ThreadPool pool(1);

    auto future = pool.submit([] {
        throw std::runtime_error("Task failed");
        return 0;
    });

    try {
        future.get();
    } catch (const std::exception & e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
}

/**
 * @brief 演示优雅关闭
 */
void demonstrateGracefulShutdown() {
    {
        ThreadPool pool(4);

        // 提交一些任务
        std::vector<std::future<void>> futures;
        for (int i = 0; i < 10; ++i) {
            futures.push_back(pool.submit([i] {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << "Task " << i << " completed\n";
            }));
        }

        // ThreadPool析构函数会等待所有任务完成
        std::cout << "Shutting down thread pool...\n";
    }
    std::cout << "All tasks completed and thread pool destroyed\n";
}

int main() {
    std::cout << "Demonstrating thread pool functionality...\n\n";

    std::cout << "1. Simple tasks:\n";
    demonstrateSimpleTasks();

    std::cout << "\n2. Long running tasks:\n";
    demonstrateLongRunningTasks();

    std::cout << "\n3. Exception handling:\n";
    demonstrateExceptionHandling();

    std::cout << "\n4. Graceful shutdown:\n";
    demonstrateGracefulShutdown();

    return 0;
}
