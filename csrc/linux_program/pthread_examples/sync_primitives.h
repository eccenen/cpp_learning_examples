#ifndef SYNC_PRIMITIVES_H
#define SYNC_PRIMITIVES_H

#include <semaphore.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <shared_mutex>

/**
 * @brief 封装了各种同步原语的示例类
 */
class SyncPrimitives {
  public:
    /**
     * @brief 互斥锁包装类
     */
    class MutexWrapper {
      public:
        void lock();
        void unlock();
        bool tryLock();
      private:
        std::mutex mutex_;
    };

    /**
     * @brief 读写锁包装类
     */
    class RWLockWrapper {
      public:
        void readLock();
        void writeLock();
        void unlock();
      private:
        std::shared_mutex rw_lock_;
    };

    /**
     * @brief 条件变量示例类
     */
    class CondVarExample {
      public:
        void produce(int item);
        int  consume();
      private:
        std::mutex              mutex_;
        std::condition_variable not_full_;
        std::condition_variable not_empty_;
        std::queue<int>         queue_;
        static constexpr size_t max_size_ = 10;
    };

    /**
     * @brief POSIX信号量包装类
     */
    class SemaphoreWrapper {
      public:
        SemaphoreWrapper(unsigned int value);
        ~SemaphoreWrapper();
        void wait();
        void post();
      private:
        sem_t sem_;
    };

    /**
     * @brief 屏障同步包装类
     */
    class BarrierWrapper {
      public:
        explicit BarrierWrapper(size_t count);
        void wait();
      private:
        std::mutex              mutex_;
        std::condition_variable cv_;
        size_t                  threshold_;
        size_t                  count_;
    };
};

#endif // SYNC_PRIMITIVES_H
