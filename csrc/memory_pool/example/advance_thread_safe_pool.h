/**
 * 高级教程1：线程安全的内存池
 *
 * 学习目标：
 * 1. 理解多线程环境下的竞争条件
 * 2. 实现细粒度锁保护
 * 3. 无锁（lock-free）内存池设计
 * 4. 性能权衡分析
 */

#ifndef THREAD_SAFE_POOL_H
#define THREAD_SAFE_POOL_H

#include <spdlog/spdlog.h>

#include <atomic>
#include <memory>
#include <mutex>

#include "../common/memory_pool_common.h"

namespace memory_pool {

/**
 * 线程安全的固定块内存池（基于互斥锁）
 */
class ThreadSafeFixedPool {
  private:
    struct Block {
        Block * next;
    };

    char *               memory_start_;
    std::atomic<Block *> free_list_; // 使用原子指针
    size_t               block_size_;
    size_t               block_count_;
    std::mutex           mutex_; // 保护统计信息
    PoolStats            stats_;

  public:
    ThreadSafeFixedPool(size_t block_size, size_t block_count) :
        memory_start_(nullptr),
        block_count_(block_count) {
        block_size  = std::max(block_size, sizeof(Block *));
        block_size_ = align_up(block_size, alignof(Block));

        size_t total_size = block_size_ * block_count_;
        memory_start_     = static_cast<char *>(::operator new(total_size));

        init_free_list();

        spdlog::info("[ThreadSafeFixedPool] 初始化: {} 块, 每块 {} bytes", block_count_,
                     block_size_);
    }

    ~ThreadSafeFixedPool() { ::operator delete(memory_start_); }

    /**
     * 线程安全的分配（使用compare-and-swap）
     */
    void * allocate() {
        Block * old_head = free_list_.load(std::memory_order_acquire);

        while (old_head != nullptr) {
            Block * new_head = old_head->next;

            // CAS操作：如果free_list_仍然是old_head，则更新为new_head
            if (free_list_.compare_exchange_weak(old_head, new_head, std::memory_order_release,
                                                 std::memory_order_acquire)) {
                // 成功分配
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    stats_.record_allocation(block_size_);
                }
                return old_head;
            }
            // CAS失败，old_head已更新，重试
        }

        return nullptr; // 内存池耗尽
    }

    /**
     * 线程安全的释放
     */
    void deallocate(void * ptr) {
        if (ptr == nullptr) {
            return;
        }

        Block * block    = static_cast<Block *>(ptr);
        Block * old_head = free_list_.load(std::memory_order_acquire);

        do {
            block->next = old_head;
        } while (!free_list_.compare_exchange_weak(old_head, block, std::memory_order_release,
                                                   std::memory_order_acquire));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            stats_.record_deallocation(block_size_);
        }
    }

    const PoolStats & stats() const { return stats_; }

    void print_status() const {
        spdlog::info("\n=== ThreadSafeFixedPool 状态 ===");
        stats_.print();
    }

  private:
    void init_free_list() {
        Block * head = nullptr;

        for (size_t i = block_count_; i > 0; --i) {
            char *  block_addr = memory_start_ + (i - 1) * block_size_;
            Block * block      = reinterpret_cast<Block *>(block_addr);
            block->next        = head;
            head               = block;
        }

        free_list_.store(head, std::memory_order_release);
    }
};

/**
 * 线程本地内存池（Thread-Local Storage）
 * 每个线程有自己的内存池，无需同步
 */
class ThreadLocalPool {
  private:
    struct PoolInstance {
        char * memory;
        void * free_list;
        size_t block_size;
        size_t block_count;

        PoolInstance(size_t bs, size_t bc) : block_size(bs), block_count(bc) {
            memory    = static_cast<char *>(::operator new(bs * bc));
            // 初始化空闲列表...
            free_list = memory;
        }

        ~PoolInstance() { ::operator delete(memory); }
    };

    size_t block_size_;
    size_t block_count_;

    // 线程局部存储
    static thread_local std::unique_ptr<PoolInstance> instance_;

  public:
    ThreadLocalPool(size_t block_size, size_t block_count) :
        block_size_(block_size),
        block_count_(block_count) {}

    void * allocate() {
        if (!instance_) {
            instance_ = std::make_unique<PoolInstance>(block_size_, block_count_);
        }
        // 在线程本地实例上分配...
        return instance_->memory; // 简化实现
    }

    void deallocate(void * ptr) {
        // 释放到线程本地池
        (void) ptr;
    }
};

// 静态成员初始化
thread_local std::unique_ptr<ThreadLocalPool::PoolInstance> ThreadLocalPool::instance_ = nullptr;

/**
 * 混合策略：线程本地缓存 + 全局池
 * 提供最佳性能和灵活性
 */
class HybridThreadPool {
  private:
    static constexpr size_t LOCAL_CACHE_SIZE = 64;

    struct LocalCache {
        void * blocks[LOCAL_CACHE_SIZE];
        size_t count = 0;
    };

    ThreadSafeFixedPool            global_pool_;
    thread_local static LocalCache cache_;

  public:
    HybridThreadPool(size_t block_size, size_t block_count) :
        global_pool_(block_size, block_count) {}

    void * allocate() {
        // 先从本地缓存分配
        if (cache_.count > 0) {
            return cache_.blocks[--cache_.count];
        }

        // 本地缓存空了，从全局池批量获取
        return global_pool_.allocate();
    }

    void deallocate(void * ptr) {
        if (ptr == nullptr) {
            return;
        }

        // 先放回本地缓存
        if (cache_.count < LOCAL_CACHE_SIZE) {
            cache_.blocks[cache_.count++] = ptr;
            return;
        }

        // 本地缓存满了，归还给全局池
        global_pool_.deallocate(ptr);
    }
};

// 线程局部缓存初始化
thread_local HybridThreadPool::LocalCache HybridThreadPool::cache_;

} // namespace memory_pool

#endif // THREAD_SAFE_POOL_H
