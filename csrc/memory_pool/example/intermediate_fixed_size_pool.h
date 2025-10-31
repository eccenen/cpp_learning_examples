// csrc/memory_pool/intermediate/04_fixed_size_pool.h
#ifndef CPP_QA_LAB_FIXED_SIZE_POOL_H_
#define CPP_QA_LAB_FIXED_SIZE_POOL_H_

#include "../common/memory_pool_common.h"

namespace memory_pool {

// ============================================================================
// 固定大小内存池实现
// 使用空闲列表（Free List）算法
// ============================================================================

class FixedSizePool {
  public:
    // 构造函数：blockSize 是每个块的大小，blockCount 是块的数量
    FixedSizePool(size_t block_size, size_t block_count);

    ~FixedSizePool();

    // 禁止拷贝
    FixedSizePool(const FixedSizePool &)             = delete;
    FixedSizePool & operator=(const FixedSizePool &) = delete;

    // 分配一个块
    void * Allocate();

    // 释放一个块
    void Deallocate(void * ptr);

    // 查询统计信息
    size_t GetBlockSize() const { return block_size_; }

    size_t GetBlockCount() const { return block_count_; }

    size_t GetUsedCount() const { return used_count_; }

    size_t GetFreeCount() const { return block_count_ - used_count_; }

    // 打印统计信息
    void PrintStats() const;

    // 可视化内存布局
    void Visualize() const;

  private:
    // 空闲列表节点
    struct FreeNode {
        FreeNode * next;
    };

    void *      memory_pool_; // 内存池基址
    FreeNode *  free_list_; // 空闲列表头
    size_t      block_size_; // 每个块的大小
    size_t      block_count_; // 块的总数
    size_t      used_count_; // 已使用的块数
    MemoryStats stats_; // 统计信息
};

} // namespace memory_pool

#endif // CPP_QA_LAB_FIXED_SIZE_POOL_H_
