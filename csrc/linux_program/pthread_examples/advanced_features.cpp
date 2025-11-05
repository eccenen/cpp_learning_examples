#include <pthread.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

// 线程局部存储示例
thread_local int thread_local_value = 0;

/**
 * @brief 清理处理函数
 */
void cleanupHandler(void * arg) {
    std::cout << "Cleanup handler called with value: " << *static_cast<int *>(arg) << std::endl;
    delete static_cast<int *>(arg);
}

/**
 * @brief 可取消的线程函数
 */
void * cancellableThread(void * arg) {
    int * value = new int(42);
    pthread_cleanup_push(cleanupHandler, value);

    while (true) {
        pthread_testcancel(); // 检查取消点
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    pthread_cleanup_pop(1);
    return nullptr;
}

/**
 * @brief 演示线程取消和清理
 */
void demonstrateCancellation() {
    pthread_t thread;
    int       rc = pthread_create(&thread, nullptr, cancellableThread, nullptr);
    if (rc) {
        std::cerr << "Error creating thread: " << rc << std::endl;
        return;
    }

    // 等待一段时间后取消线程
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    pthread_cancel(thread);

    // 等待线程结束
    pthread_join(thread, nullptr);
}

/**
 * @brief 演示线程属性设置
 */
void demonstrateThreadAttributes() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 设置栈大小
    size_t stacksize;
    pthread_attr_getstacksize(&attr, &stacksize);
    std::cout << "Default stack size: " << stacksize << " bytes\n";

    // 设置新的栈大小（2MB）
    pthread_attr_setstacksize(&attr, 2 * 1024 * 1024);

    // 设置分离状态
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_attr_destroy(&attr);
}

/**
 * @brief 演示实时线程优先级
 */
void demonstrateRealTimeThreads() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 设置调度策略为实时调度
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    // 设置优先级
    sched_param param;
    param.sched_priority = 50; // 中等优先级
    pthread_attr_setschedparam(&attr, &param);

    pthread_attr_destroy(&attr);
}

/**
 * @brief 演示线程局部存储
 */
void demonstrateThreadLocalStorage() {
    auto threadFunc = [](int id) {
        thread_local_value = id;
        std::cout << "Thread " << id << " TLS value: " << thread_local_value << std::endl;
    };

    std::thread t1(threadFunc, 1);
    std::thread t2(threadFunc, 2);
    std::thread t3(threadFunc, 3);

    t1.join();
    t2.join();
    t3.join();
}

int main() {
    std::cout << "Demonstrating advanced thread features...\n\n";

    std::cout << "1. Thread cancellation and cleanup:\n";
    demonstrateCancellation();

    std::cout << "\n2. Thread attributes:\n";
    demonstrateThreadAttributes();

    std::cout << "\n3. Real-time thread priorities:\n";
    demonstrateRealTimeThreads();

    std::cout << "\n4. Thread local storage:\n";
    demonstrateThreadLocalStorage();

    return 0;
}
