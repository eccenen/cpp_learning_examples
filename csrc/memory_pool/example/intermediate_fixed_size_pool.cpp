// csrc/memory_pool/intermediate/04_fixed_size_pool.cpp
#include "../common/visualizer.h"
#include "intermediate_fixed_size_pool.h"

namespace memory_pool {

FixedSizePool::FixedSizePool(size_t block_size, size_t block_count) :
    block_size_(std::max(block_size, sizeof(FreeNode))),
    block_count_(block_count),
    used_count_(0) {
    // 分配整个内存池
    size_t total_size = block_size_ * block_count_;
    memory_pool_      = std::malloc(total_size);

    if (!memory_pool_) {
        throw std::bad_alloc();
    }

    stats_.recordAllocation(total_size);

    // 初始化空闲列表
    free_list_ = static_cast<FreeNode *>(memory_pool_);

    char * current = static_cast<char *>(memory_pool_);
    for (size_t i = 0; i < block_count_ - 1; ++i) {
        FreeNode * node = reinterpret_cast<FreeNode *>(current);
        node->next      = reinterpret_cast<FreeNode *>(current + block_size_);
        current += block_size_;
    }

    // 最后一个节点指向 nullptr
    FreeNode * last_node = reinterpret_cast<FreeNode *>(current);
    last_node->next      = nullptr;

    spdlog::info("FixedSizePool 初始化：块大小={} bytes, 数量={}, 总大小={} bytes", block_size_,
                 block_count_, total_size);
}

FixedSizePool::~FixedSizePool() {
    if (used_count_ > 0) {
        spdlog::warn("⚠ 内存池销毁时仍有 {} 个块未释放！", used_count_);
    }

    stats_.recordDeallocation(block_size_ * block_count_);
    std::free(memory_pool_);

    spdlog::info("FixedSizePool 销毁");
}

void * FixedSizePool::allocate() {
    if (!free_list_) {
        spdlog::error("❌ 内存池已耗尽！");
        return nullptr;
    }

    // 从空闲列表头部取出一个节点
    FreeNode * node = free_list_;
    free_list_      = node->next;
    used_count_++;

    stats_.recordAllocation(block_size_);

    return static_cast<void *>(node);
}

void FixedSizePool::deallocate(void * ptr) {
    if (!ptr) {
        return;
    }

    // 检查指针是否在内存池范围内
    char * char_ptr   = static_cast<char *>(ptr);
    char * pool_start = static_cast<char *>(memory_pool_);
    char * pool_end   = pool_start + (block_size_ * block_count_);

    if (char_ptr < pool_start || char_ptr >= pool_end) {
        spdlog::error("❌ 尝试释放不属于此内存池的指针！");
        return;
    }

    // 将节点归还到空闲列表头部
    FreeNode * node = static_cast<FreeNode *>(ptr);
    node->next      = free_list_;
    free_list_      = node;
    used_count_--;

    stats_.recordDeallocation(block_size_);
}

void FixedSizePool::printStats() const {
    spdlog::info("\n=== FixedSizePool 统计信息 ===");
    spdlog::info("块大小: {} bytes", block_size_);
    spdlog::info("总块数: {}", block_count_);
    spdlog::info("已使用: {}", used_count_);
    spdlog::info("空闲: {}", GetFreeCount());
    spdlog::info("使用率: {:.1f}%", (used_count_ * 100.0) / block_count_);
    stats_.show();
}

void FixedSizePool::visualize() const {
    Memoryvisualizer::visualizePoolLayout(memory_pool_, block_size_, block_count_);
    Memoryvisualizer::visualizeFreeList(free_list_, "FixedSizePool");
}

} // namespace memory_pool
