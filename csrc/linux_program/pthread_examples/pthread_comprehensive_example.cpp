/**
 * @file pthread_comprehensive_example.cpp
 * @brief Linux pthread编程全面示例
 * @details 本示例展示了pthread的各种使用方法，包括常见错误和最佳实践
 *
 * 编译: g++ -o pthread_example pthread_comprehensive_example.cpp -lpthread -std=c++17
 */

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <cstdint>

// ============================================================================
// 第一部分：基础线程操作
// ============================================================================

/**
 * @brief 线程参数结构体
 */
struct ThreadArgs {
    int          thread_id;
    const char * message;
    int *        result;
};

/**
 * @brief 简单的线程函数
 * @param arg 线程参数指针
 * @return 线程返回值
 */
void * simpleThreadFunc(void * arg) {
    ThreadArgs * args = static_cast<ThreadArgs *>(arg);
    printf("[线程 %d] 开始执行: %s\n", args->thread_id, args->message);

    // 模拟工作
    sleep(1);

    // 设置返回值
    if (args->result) {
        *(args->result) = args->thread_id * 100;
    }

    printf("[线程 %d] 执行完成\n", args->thread_id);
    return args->result;
}

/**
 * @brief 演示基础线程创建和等待
 */
void demonstrateBasicThread() {
    printf("\n========== 示例1: 基础线程操作 ==========\n");

    pthread_t  thread;
    int        result = 0;
    ThreadArgs args   = { 1, "Hello from thread", &result };

    // 创建线程
    int rc = pthread_create(&thread, nullptr, simpleThreadFunc, &args);
    if (rc != 0) {
        printf("错误: 线程创建失败, 错误码=%d\n", rc);
        return;
    }

    printf("[主线程] 线程已创建，等待完成...\n");

    // 等待线程结束并获取返回值
    void * ret_val;
    rc = pthread_join(thread, &ret_val);
    if (rc != 0) {
        printf("错误: pthread_join失败, 错误码=%d\n", rc);
        return;
    }

    printf("[主线程] 线程返回值: %d\n", result);
    printf("[子线程] return值: %d\n", *(uint32_t *) ret_val);
}

// ============================================================================
// 第二部分：线程分离
// ============================================================================

void * detachedThreadFunc(void * arg) {
    int id = *static_cast<int *>(arg);
    printf("[分离线程 %d] 运行中...\n", id);
    sleep(1);
    printf("[分离线程 %d] 完成\n", id);
    delete static_cast<int *>(arg); // 清理动态分配的内存
    return nullptr;
}

/**
 * @brief 演示线程分离
 */
void demonstrateDetachedThread() {
    printf("\n========== 示例2: 线程分离 ==========\n");

    pthread_t thread;
    int *     id = new int(2);

    // 创建线程
    pthread_create(&thread, nullptr, detachedThreadFunc, id);

    // 分离线程 - 线程结束后会自动释放资源
    int rc = pthread_detach(thread);
    if (rc != 0) {
        printf("错误: pthread_detach失败\n");
        return;
    }

    printf("[主线程] 线程已分离，无需等待\n");
    sleep(2); // 等待分离线程完成
}

// ============================================================================
// 第三部分：互斥锁（Mutex）
// ============================================================================

// 全局共享数据和互斥锁
int             shared_counter = 0;
pthread_mutex_t counter_mutex  = PTHREAD_MUTEX_INITIALIZER;

void * incrementCounter(void * arg) {
    int id = *static_cast<int *>(arg);

    for (int i = 0; i < 1000; ++i) {
        // 加锁
        pthread_mutex_lock(&counter_mutex);

        // 临界区
        int temp = shared_counter;
        temp++;
        shared_counter = temp;

        // 解锁
        pthread_mutex_unlock(&counter_mutex);
    }

    printf("[线程 %d] 完成1000次增量操作\n", id);
    return nullptr;
}

/**
 * @brief 演示互斥锁的使用
 */
void demonstrateMutex() {
    printf("\n========== 示例3: 互斥锁（Mutex） ==========\n");

    shared_counter        = 0;
    const int num_threads = 5;
    pthread_t threads[num_threads];
    int       thread_ids[num_threads];

    // 创建多个线程
    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, incrementCounter, &thread_ids[i]);
    }

    // 等待所有线程完成
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    printf("[主线程] 最终计数器值: %d (期望: %d)\n", shared_counter, num_threads * 1000);

    // 清理互斥锁
    pthread_mutex_destroy(&counter_mutex);
}

// ============================================================================
// 第四部分：条件变量（Condition Variable）
// ============================================================================

/**
 * @brief 生产者-消费者队列
 */
struct Queue {
    int             buffer[10];
    int             count;
    int             in;
    int             out;
    pthread_mutex_t mutex;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
};

Queue queue = {
    {}, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER
};

void * producer(void * arg) {
    int id = *static_cast<int *>(arg);

    for (int i = 0; i < 5; ++i) {
        pthread_mutex_lock(&queue.mutex);

        // 等待队列有空间
        while (queue.count >= 10) {
            printf("[生产者 %d] 队列已满，等待...\n", id);
            pthread_cond_wait(&queue.not_full, &queue.mutex);
        }

        // 生产数据
        int item               = id * 100 + i;
        queue.buffer[queue.in] = item;
        queue.in               = (queue.in + 1) % 10;
        queue.count++;

        printf("[生产者 %d] 生产: %d (队列大小: %d)\n", id, item, queue.count);

        // 通知消费者
        pthread_cond_signal(&queue.not_empty);

        pthread_mutex_unlock(&queue.mutex);

        usleep(100000); // 100ms
    }

    return nullptr;
}

void * consumer(void * arg) {
    int id = *static_cast<int *>(arg);

    for (int i = 0; i < 10; ++i) {
        pthread_mutex_lock(&queue.mutex);

        // 等待队列有数据
        while (queue.count == 0) {
            printf("[消费者 %d] 队列为空，等待...\n", id);
            pthread_cond_wait(&queue.not_empty, &queue.mutex);
        }

        // 消费数据
        int item  = queue.buffer[queue.out];
        queue.out = (queue.out + 1) % 10;
        queue.count--;

        printf("[消费者 %d] 消费: %d (队列大小: %d)\n", id, item, queue.count);

        // 通知生产者
        pthread_cond_signal(&queue.not_full);

        pthread_mutex_unlock(&queue.mutex);

        usleep(150000); // 150ms
    }

    return nullptr;
}

/**
 * @brief 演示条件变量（生产者-消费者模式）
 */
void demonstrateConditionVariable() {
    printf("\n========== 示例4: 条件变量（生产者-消费者） ==========\n");

    pthread_t prod_thread, cons_thread;
    int       prod_id = 1, cons_id = 1;

    pthread_create(&prod_thread, nullptr, producer, &prod_id);
    pthread_create(&cons_thread, nullptr, consumer, &cons_id);

    pthread_join(prod_thread, nullptr);
    pthread_join(cons_thread, nullptr);

    printf("[主线程] 生产者-消费者示例完成\n");

    // 清理资源
    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.not_empty);
    pthread_cond_destroy(&queue.not_full);
}

// ============================================================================
// 第五部分：读写锁（Read-Write Lock）
// ============================================================================

int              shared_data = 0;
pthread_rwlock_t rwlock      = PTHREAD_RWLOCK_INITIALIZER;

void * reader(void * arg) {
    int id = *static_cast<int *>(arg);

    for (int i = 0; i < 3; ++i) {
        // 获取读锁
        pthread_rwlock_rdlock(&rwlock);

        printf("[读者 %d] 读取数据: %d\n", id, shared_data);
        usleep(100000);

        // 释放读锁
        pthread_rwlock_unlock(&rwlock);

        usleep(200000);
    }

    return nullptr;
}

void * writer(void * arg) {
    int id = *static_cast<int *>(arg);

    for (int i = 0; i < 2; ++i) {
        // 获取写锁
        pthread_rwlock_wrlock(&rwlock);

        shared_data = id * 10 + i;
        printf("[写者 %d] 写入数据: %d\n", id, shared_data);
        usleep(200000);

        // 释放写锁
        pthread_rwlock_unlock(&rwlock);

        usleep(300000);
    }

    return nullptr;
}

/**
 * @brief 演示读写锁
 */
void demonstrateReadWriteLock() {
    printf("\n========== 示例5: 读写锁 ==========\n");

    const int num_readers = 3;
    const int num_writers = 2;
    pthread_t reader_threads[num_readers];
    pthread_t writer_threads[num_writers];
    int       reader_ids[num_readers];
    int       writer_ids[num_writers];

    // 创建读者线程
    for (int i = 0; i < num_readers; ++i) {
        reader_ids[i] = i;
        pthread_create(&reader_threads[i], nullptr, reader, &reader_ids[i]);
    }

    // 创建写者线程
    for (int i = 0; i < num_writers; ++i) {
        writer_ids[i] = i;
        pthread_create(&writer_threads[i], nullptr, writer, &writer_ids[i]);
    }

    // 等待所有线程完成
    for (int i = 0; i < num_readers; ++i) {
        pthread_join(reader_threads[i], nullptr);
    }
    for (int i = 0; i < num_writers; ++i) {
        pthread_join(writer_threads[i], nullptr);
    }

    printf("[主线程] 读写锁示例完成\n");
    pthread_rwlock_destroy(&rwlock);
}

// ============================================================================
// 第六部分：信号量（Semaphore）
// ============================================================================

sem_t semaphore;

void * semaphoreWorker(void * arg) {
    int id = *static_cast<int *>(arg);

    for (int i = 0; i < 2; ++i) {
        printf("[工作线程 %d] 等待信号量...\n", id);
        sem_wait(&semaphore);

        printf("[工作线程 %d] 进入临界区 (轮次 %d)\n", id, i);
        sleep(1);
        printf("[工作线程 %d] 离开临界区 (轮次 %d)\n", id, i);

        sem_post(&semaphore);
        usleep(100000);
    }

    return nullptr;
}

/**
 * @brief 演示信号量
 */
void demonstrateSemaphore() {
    printf("\n========== 示例6: 信号量 ==========\n");

    // 初始化信号量，初始值为2（允许2个线程同时访问）
    sem_init(&semaphore, 0, 2);

    const int num_threads = 4;
    pthread_t threads[num_threads];
    int       thread_ids[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, semaphoreWorker, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    printf("[主线程] 信号量示例完成\n");
    sem_destroy(&semaphore);
}

// ============================================================================
// 第七部分：线程属性设置
// ============================================================================

/**
 * @brief 演示线程属性设置
 */
void demonstrateThreadAttributes() {
    printf("\n========== 示例7: 线程属性 ==========\n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 1. 获取和设置栈大小
    size_t stack_size;
    pthread_attr_getstacksize(&attr, &stack_size);
    printf("[属性] 默认栈大小: %zu 字节\n", stack_size);

    // 设置新的栈大小为2MB
    pthread_attr_setstacksize(&attr, 2 * 1024 * 1024);
    pthread_attr_getstacksize(&attr, &stack_size);
    printf("[属性] 新栈大小: %zu 字节\n", stack_size);

    // 2. 设置分离状态
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int detach_state;
    pthread_attr_getdetachstate(&attr, &detach_state);
    printf("[属性] 分离状态: %s\n", detach_state == PTHREAD_CREATE_DETACHED ? "分离" : "可连接");

    // 3. 设置调度策略（需要root权限）
    int policy;
    pthread_attr_getschedpolicy(&attr, &policy);
    printf("[属性] 调度策略: ");
    switch (policy) {
        case SCHED_OTHER:
            printf("SCHED_OTHER (默认)\n");
            break;
        case SCHED_FIFO:
            printf("SCHED_FIFO (实时FIFO)\n");
            break;
        case SCHED_RR:
            printf("SCHED_RR (实时轮转)\n");
            break;
        default:
            printf("未知\n");
    }

    // 清理属性对象
    pthread_attr_destroy(&attr);
}

// ============================================================================
// 第八部分：线程取消
// ============================================================================

void * cancellableThreadFunc(void * arg) {
    int id = *static_cast<int *>(arg);

    // 设置线程可取消
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);

    printf("[可取消线程 %d] 开始执行\n", id);

    for (int i = 0; i < 10; ++i) {
        printf("[可取消线程 %d] 循环 %d/10\n", id, i + 1);
        sleep(1);

        // 设置取消点
        pthread_testcancel();
    }

    printf("[可取消线程 %d] 正常结束\n", id);
    return nullptr;
}

/**
 * @brief 演示线程取消
 */
void demonstrateThreadCancellation() {
    printf("\n========== 示例8: 线程取消 ==========\n");

    pthread_t thread;
    int       id = 8;

    pthread_create(&thread, nullptr, cancellableThreadFunc, &id);

    // 等待3秒后取消线程
    sleep(3);
    printf("[主线程] 发送取消请求\n");
    pthread_cancel(thread);

    // 等待线程结束
    void * result;
    pthread_join(thread, &result);

    if (result == PTHREAD_CANCELED) {
        printf("[主线程] 线程已被取消\n");
    } else {
        printf("[主线程] 线程正常结束\n");
    }
}

// ============================================================================
// 第九部分：线程局部存储（Thread-Local Storage）
// ============================================================================

pthread_key_t tls_key;

void tlsDestructor(void * value) {
    printf("[析构函数] 清理TLS数据: %d\n", *static_cast<int *>(value));
    delete static_cast<int *>(value);
}

void * tlsThreadFunc(void * arg) {
    int id = *static_cast<int *>(arg);

    // 为每个线程设置不同的TLS值
    int * tls_value = new int(id * 100);
    pthread_setspecific(tls_key, tls_value);

    printf("[线程 %d] TLS值: %d\n", id, *static_cast<int *>(pthread_getspecific(tls_key)));

    sleep(1);

    // TLS值在线程退出时会被自动清理（调用析构函数）
    return nullptr;
}

/**
 * @brief 演示线程局部存储
 */
void demonstrateThreadLocalStorage() {
    printf("\n========== 示例9: 线程局部存储 ==========\n");

    // 创建TLS键
    pthread_key_create(&tls_key, tlsDestructor);

    const int num_threads = 3;
    pthread_t threads[num_threads];
    int       thread_ids[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, tlsThreadFunc, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    printf("[主线程] TLS示例完成\n");
    pthread_key_delete(tls_key);
}

// ============================================================================
// 第十部分：常见错误示例
// ============================================================================

/**
 * @brief 演示常见的pthread编程错误
 * @warning 这些是错误示例，仅用于教学目的
 */
void demonstrateCommonMistakes() {
    printf("\n========== 示例10: 常见错误（仅供参考，已修正） ==========\n");

    printf("\n【错误1】忘记初始化互斥锁\n");
    printf("修正: 使用PTHREAD_MUTEX_INITIALIZER或pthread_mutex_init()\n");

    printf("\n【错误2】死锁 - 重复加锁同一个互斥锁\n");
    printf("修正: 使用pthread_mutex_trylock()或递归互斥锁\n");

    printf("\n【错误3】忘记解锁\n");
    printf("修正: 确保每个lock()都有对应的unlock()，使用RAII模式\n");

    printf("\n【错误4】在持有锁时调用可能阻塞的函数\n");
    printf("修正: 尽量缩小临界区范围\n");

    printf("\n【错误5】条件变量使用if而不是while检查条件\n");
    printf("修正: 使用while循环防止虚假唤醒\n");
    pthread_mutex_t m         = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t  c         = PTHREAD_COND_INITIALIZER;
    bool            condition = false;

    pthread_mutex_lock(&m);
    // 错误: if (condition) { ... }
    // 正确:
    while (!condition) {
        pthread_cond_wait(&c, &m);
    }
    pthread_mutex_unlock(&m);

    printf("\n【错误6】忘记调用pthread_join或pthread_detach\n");
    printf("修正: 每个线程都需要join或detach，否则会资源泄漏\n");

    printf("\n【错误7】线程函数返回后访问栈上的数据\n");
    printf("修正: 使用堆分配或全局变量\n");

    printf("\n【错误8】没有检查函数返回值\n");
    printf("修正: 始终检查pthread函数的返回值\n");

    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&c);
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    printf("====================================================\n");
    printf("      Linux pthread 编程全面示例\n");
    printf("====================================================\n");

    // 执行所有示例
    demonstrateBasicThread();
    demonstrateDetachedThread();
    demonstrateMutex();
    demonstrateConditionVariable();
    demonstrateReadWriteLock();
    demonstrateSemaphore();
    demonstrateThreadAttributes();
    demonstrateThreadCancellation();
    demonstrateThreadLocalStorage();
    demonstrateCommonMistakes();

    printf("\n====================================================\n");
    printf("      所有示例执行完成\n");
    printf("====================================================\n");

    return 0;
}
