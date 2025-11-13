#include <pthread.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

/**
 * @brief 线程函数参数结构体
 */
struct ThreadArgs {
    int         thread_id;
    std::string message;
};

/**
 * @brief 线程函数返回值结构体
 */
struct ThreadResult {
    int thread_id;
    int result;
};

/**
 * @brief 简单的线程函数
 * @param arg 线程参数
 * @return 线程返回值
 */
void * threadFunction(void * arg) {
    auto * args = static_cast<ThreadArgs *>(arg);
    std::cout << "Thread " << args->thread_id << ": " << args->message << std::endl;

    // 模拟一些工作
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 创建返回值
    auto * result = new ThreadResult{ args->thread_id, args->thread_id * 2 };
    return result;
}

/**
 * @brief 演示线程分离
 */
void demonstrateDetach() {
    pthread_t thread;
    auto *    args = new ThreadArgs{ 1, "This is a detached thread" };

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int rc = pthread_create(&thread, &attr, threadFunction, args);
    if (rc) {
        std::cerr << "Error creating thread: " << rc << std::endl;
        exit(-1);
    }

    pthread_attr_destroy(&attr);
    // 无需 pthread_join，线程会自动清理
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

/**
 * @brief 演示线程连接和返回值获取
 */
void demonstrateJoin() {
    pthread_t thread;
    auto *    args = new ThreadArgs{ 2, "This is a joinable thread" };

    int rc = pthread_create(&thread, nullptr, threadFunction, args);
    if (rc) {
        std::cerr << "Error creating thread: " << rc << std::endl;
        exit(-1);
    }

    // 等待线程完成并获取返回值
    void * thread_result;
    rc = pthread_join(thread, &thread_result);
    if (rc) {
        std::cerr << "Error joining thread: " << rc << std::endl;
        exit(-1);
    }

    auto * result = static_cast<ThreadResult *>(thread_result);
    std::cout << "Thread " << result->thread_id << " returned: " << result->result << std::endl;

    delete result;
    delete args;
}

int main() {
    std::cout << "Demonstrating basic thread operations...\n\n";

    std::cout << "1. Thread detach demonstration:\n";
    demonstrateDetach();

    std::cout << "\n2. Thread join demonstration:\n";
    demonstrateJoin();

    return 0;
}
