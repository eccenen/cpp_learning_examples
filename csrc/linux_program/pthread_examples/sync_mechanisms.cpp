#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "sync_primitives.h"

/**
 * @brief 演示互斥锁的使用
 */
void demonstrateMutex() {
    SyncPrimitives::MutexWrapper mutex;
    int                          shared_counter = 0;

    auto increment = [&](int id) {
        for (int i = 0; i < 1000; ++i) {
            mutex.lock();
            ++shared_counter;
            mutex.unlock();
        }
        std::cout << "Thread " << id << " finished incrementing\n";
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(increment, i);
    }

    for (auto & t : threads) {
        t.join();
    }

    std::cout << "Final counter value: " << shared_counter << "\n";
}

/**
 * @brief 演示读写锁的使用
 */
void demonstrateRWLock() {
    SyncPrimitives::RWLockWrapper rwlock;
    int                           shared_data = 0;

    auto reader = [&](int id) {
        for (int i = 0; i < 3; ++i) {
            rwlock.readLock();
            std::cout << "Reader " << id << " read value: " << shared_data << "\n";
            rwlock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto writer = [&](int id) {
        for (int i = 0; i < 2; ++i) {
            rwlock.writeLock();
            shared_data = id * 10 + i;
            std::cout << "Writer " << id << " wrote value: " << shared_data << "\n";
            rwlock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(reader, i);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(writer, i);
    }

    for (auto & t : threads) {
        t.join();
    }
}

/**
 * @brief 演示条件变量的使用（生产者-消费者模式）
 */
void demonstrateCondVar() {
    SyncPrimitives::CondVarExample pc;

    auto producer = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            int item = id * 10 + i;
            pc.produce(item);
            std::cout << "Producer " << id << " produced: " << item << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto consumer = [&](int id) {
        for (int i = 0; i < 10; ++i) {
            int item = pc.consume();
            std::cout << "Consumer " << id << " consumed: " << item << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    };

    std::thread p1(producer, 1);
    std::thread p2(producer, 2);
    std::thread c(consumer, 1);

    p1.join();
    p2.join();
    c.join();
}

/**
 * @brief 演示信号量的使用
 */
void demonstrateSemaphore() {
    SyncPrimitives::SemaphoreWrapper sem(2); // 允许2个并发访问

    auto worker = [&](int id) {
        for (int i = 0; i < 3; ++i) {
            sem.wait();
            std::cout << "Thread " << id << " entered critical section\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << "Thread " << id << " leaving critical section\n";
            sem.post();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto & t : threads) {
        t.join();
    }
}

/**
 * @brief 演示屏障同步的使用
 */
void demonstrateBarrier() {
    constexpr int                  num_threads = 3;
    SyncPrimitives::BarrierWrapper barrier(num_threads);

    auto worker = [&](int id) {
        for (int phase = 0; phase < 2; ++phase) {
            std::cout << "Thread " << id << " phase " << phase << " started\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
            std::cout << "Thread " << id << " waiting at barrier\n";
            barrier.wait();
            std::cout << "Thread " << id << " passed barrier\n";
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto & t : threads) {
        t.join();
    }
}

int main() {
    std::cout << "Demonstrating synchronization mechanisms...\n\n";

    std::cout << "1. Mutex demonstration:\n";
    demonstrateMutex();

    std::cout << "\n2. Read-Write Lock demonstration:\n";
    demonstrateRWLock();

    std::cout << "\n3. Condition Variable demonstration:\n";
    demonstrateCondVar();

    std::cout << "\n4. Semaphore demonstration:\n";
    demonstrateSemaphore();

    std::cout << "\n5. Barrier demonstration:\n";
    demonstrateBarrier();

    return 0;
}
