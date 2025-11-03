/**
 * 中级教程1：单链表管理的固定块内存池
 *
 * 学习目标：
 * 1. 理解固定大小内存池的原理
 * 2. 掌握空闲列表（Free List）算法
 * 3. 实现O(1)的分配和释放
 * 4. 理解内存对齐的重要性
 *
 * 内存布局：
 * ┌────────┬────────┬────────┬────────┐
 * │ Block0 │ Block1 │ Block2 │ Block3 │
 * └────────┴────────┴────────┴────────┘
 *     │        │        │
 *     └────────┴────────┴───> nullptr
 *   (Free List)
 */

#ifndef FIXED_BLOCK_POOL_H
#define FIXED_BLOCK_POOL_H

#include "../common/memory_pool_common.h"
#include "common.h"

namespace memory_pool {

/**
 * 固定块内存池
 * 所有分配的块大小相同，使用单链表管理空闲块
 */
class FixedBlockPool {
  private:
    // 内存块头部（当块空闲时存储下一个空闲块的指针）
    union Block {
        Block * next; // 空闲时：指向下一个空闲块
        alignas(std::max_align_t) char data[1]; // 使用时：用户数据
    };

    char *      memory_start_; // 内存池起始地址
    Block *     free_list_; // 空闲块链表头
    size_t      block_size_; // 每个块的大小（含头部）
    size_t      block_count_; // 块的总数
    MemoryStats stats_; // 统计信息

  public:
    /**
     * 构造固定块内存池
     * @param block_size 每个块的大小（字节）
     * @param block_count 块的数量
     */
    FixedBlockPool(size_t block_size, size_t block_count) :
        memory_start_(nullptr),
        free_list_(nullptr),
        block_count_(block_count) {
        // 确保块大小至少能容纳一个指针
        block_size = std::max(block_size, sizeof(Block *));

        // 对齐到指针大小
        block_size_ = alignUp(block_size, alignof(Block));

        // 分配整块内存
        size_t total_size = block_size_ * block_count_;
        memory_start_     = static_cast<char *>(::operator new(total_size));

        spdlog::info("[FixedBlockPool] 初始化:");
        spdlog::info("  块大小: {} bytes", block_size_);
        spdlog::info("  块数量: {}", block_count_);
        spdlog::info("  总大小: {} bytes", total_size);
        spdlog::info("  起始地址: {}", static_cast<void *>(memory_start_));

        // 初始化空闲列表
        init_free_list();
    }

    ~FixedBlockPool() {
        if (stats_.current_usage.load() > 0) {
            spdlog::warn("[警告] 内存池销毁时还有 {} bytes未释放", stats_.current_usage.load());
        }

        ::operator delete(memory_start_);
        spdlog::info("[FixedBlockPool] 销毁");
    }

    // 禁止拷贝
    FixedBlockPool(const FixedBlockPool &)             = delete;
    FixedBlockPool & operator=(const FixedBlockPool &) = delete;

    /**
     * 分配一个内存块
     * @return 指向分配的内存块的指针，失败返回nullptr
     */
    void * allocate() {
        if (free_list_ == nullptr) {
            spdlog::error("[错误] 内存池耗尽！");
            return nullptr;
        }

        // 从空闲列表头部取出一块
        Block * block = free_list_;
        free_list_    = block->next;

        // 更新统计
        stats_.recordAllocation(block_size_);

        return block;
    }

    /**
     * 释放一个内存块
     * @param ptr 要释放的内存块指针
     */
    void deallocate(void * ptr) {
        if (ptr == nullptr) {
            return;
        }

        // 检查指针是否属于这个池
        if (!owns(ptr)) {
            spdlog::error("[错误] 试图释放不属于此池的内存: {}", ptr);
            return;
        }

        // 将块加入空闲列表头部
        Block * block = static_cast<Block *>(ptr);
        block->next   = free_list_;
        free_list_    = block;

        // 更新统计
        stats_.recordDeallocation(block_size_);
    }

    /**
     * 检查指针是否属于这个内存池
     */
    bool owns(void * ptr) const {
        uintptr_t p     = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t start = reinterpret_cast<uintptr_t>(memory_start_);
        uintptr_t end   = start + block_size_ * block_count_;
        return p >= start && p < end;
    }

    /**
     * 获取统计信息
     */
    const MemoryStats & stats() const { return stats_; }

    /**
     * 打印内存池状态
     */
    void print_status() const {
        spdlog::info("\n=== FixedBlockPool 状态 ===");
        spdlog::info("块大小: {} bytes", block_size_);
        spdlog::info("总块数: {}", block_count_);

        // 计算空闲块数量
        size_t  free_count = 0;
        Block * current    = free_list_;
        while (current != nullptr) {
            free_count++;
            current = current->next;
        }

        spdlog::info("空闲块: {}", free_count);
        spdlog::info("已用块: {}", (block_count_ - free_count));
        spdlog::info("使用率: {}%", (100.0 * (block_count_ - free_count) / block_count_));

        stats_.show();
    }

    /**
     * 可视化内存布局
     */
    void visualize() const {
        spdlog::info("\n=== 内存布局可视化 ===");
        spdlog::info("内存起始: {}", static_cast<void *>(memory_start_));

        // 标记哪些块是空闲的
        std::vector<bool> is_free(block_count_, false);
        Block *           current = free_list_;
        while (current != nullptr) {
            size_t index = (reinterpret_cast<char *>(current) - memory_start_) / block_size_;
            if (index < block_count_) {
                is_free[index] = true;
            }
            current = current->next;
        }

        // 打印布局
        for (size_t i = 0; i < block_count_; ++i) {
            if (i % 10 == 0) {
                spdlog::info("\n{}:", i);
            }
            // 为简单起见，将符号记录为info
            spdlog::info("{}", (is_free[i] ? "□ " : "■ "));
        }
        spdlog::info("\n\n■ = 已分配  □ = 空闲");
    }

  private:
    /**
     * 初始化空闲列表
     * 将所有块链接成单链表
     */
    void initFreeList() {
        free_list_ = nullptr;

        // 从后向前链接，这样第一次分配就是从前往后
        for (size_t i = block_count_; i > 0; --i) {
            char *  block_addr = memory_start_ + (i - 1) * block_size_;
            Block * block      = reinterpret_cast<Block *>(block_addr);
            block->next        = free_list_;
            free_list_         = block;
        }
    }

    // 兼容旧命名
    void init_free_list() { initFreeList(); }
};

} // namespace memory_pool

#endif // FIXED_BLOCK_POOL_H
