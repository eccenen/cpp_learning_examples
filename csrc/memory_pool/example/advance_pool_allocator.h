/**
 * 高级教程2：STL兼容的内存池分配器
 *
 * 学习目标：
 * 1. 理解STL分配器接口
 * 2. 实现符合C++11/14/17标准的分配器
 * 3. 与std::vector、std::list等容器集成
 * 4. 处理rebind机制
 */

#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include <spdlog/spdlog.h>

#include <iostream>
#include <limits>
#include <memory>

#include "intermediate_fixed_block_pool.h"

namespace memory_pool {

/**
 * STL兼容的池分配器
 * 可用于std::vector, std::list等标准容器
 */
template <typename T> class PoolAllocator {
  public:
    // STL分配器必需的类型定义
    using value_type                             = T;
    using size_type                              = std::size_t;
    using difference_type                        = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal                        = std::false_type;

  private:
    FixedBlockPool * pool_; // 指向共享的内存池
    bool             owns_pool_; // 是否拥有池的所有权

  public:
    /**
     * 默认构造：创建自己的池
     */
    PoolAllocator(size_t block_count = 1024) :
        pool_(new FixedBlockPool(sizeof(T), block_count)),
        owns_pool_(true) {
        spdlog::info("[PoolAllocator] 创建新池，块大小={}, 数量={}", sizeof(T), block_count);
    }

    /**
     * 共享池构造：使用外部池
     */
    explicit PoolAllocator(FixedBlockPool * pool) : pool_(pool), owns_pool_(false) {}

    /**
     * 拷贝构造：共享池
     */
    PoolAllocator(const PoolAllocator & other) noexcept : pool_(other.pool_), owns_pool_(false) {}

    /**
     * Rebind拷贝构造（从U类型转换到T类型）
     */
    template <typename U>
    PoolAllocator(const PoolAllocator<U> & other) noexcept :
        pool_(other.get_pool()),
        owns_pool_(false) {}

    /**
     * 析构函数
     */
    ~PoolAllocator() {
        if (owns_pool_) {
            delete pool_;
        }
    }

    /**
     * 分配n个T对象的内存
     */
    T * allocate(size_type n) {
        if (n == 0) {
            return nullptr;
        }

        if (n > max_size()) {
            throw std::bad_array_new_length();
        }

        // 如果请求大小与块大小匹配，使用池分配
        if (n == 1) {
            void * ptr = pool_->allocate();
            if (!ptr) {
                throw std::bad_alloc();
            }
            return static_cast<T *>(ptr);
        }

        // 否则回退到标准分配
        spdlog::warn("[警告] 分配大小({})不匹配块大小，使用标准分配", n);
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }

    /**
     * 释放内存
     */
    void deallocate(T * ptr, size_type n) {
        if (ptr == nullptr) {
            return;
        }

        // 检查是否属于池
        if (n == 1 && pool_->owns(ptr)) {
            pool_->deallocate(ptr);
        } else {
            ::operator delete(ptr);
        }
    }

    /**
     * 返回可分配的最大对象数
     */
    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    /**
     * 获取底层池（用于rebind）
     */
    FixedBlockPool * get_pool() const { return pool_; }

    /**
     * Rebind：允许分配器为不同类型分配内存
     * 这是STL容器（如list）需要的，因为它们可能需要为内部节点分配内存
     */
    template <typename U> struct rebind {
        using other = PoolAllocator<U>;
    };

    /**
     * 相等比较：如果使用相同的池则相等
     */
    bool operator==(const PoolAllocator & other) const noexcept { return pool_ == other.pool_; }

    bool operator!=(const PoolAllocator & other) const noexcept { return !(*this == other); }
};

/**
 * 简化的分配器（不需要rebind，仅用于示例）
 */
template <typename T> class SimplePoolAllocator {
  public:
    using value_type = T;

    SimplePoolAllocator() = default;

    template <typename U> SimplePoolAllocator(const SimplePoolAllocator<U> &) {}

    T * allocate(std::size_t n) {
        spdlog::info("[SimplePoolAllocator] 分配 {} 个 {}", n, typeid(T).name());
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }

    void deallocate(T * ptr, std::size_t n) {
        spdlog::info("[SimplePoolAllocator] 释放 {} 个 {}", n, typeid(T).name());
        ::operator delete(ptr);
    }
};

template <typename T, typename U>
bool operator==(const SimplePoolAllocator<T> &, const SimplePoolAllocator<U> &) {
    return true;
}

template <typename T, typename U>
bool operator!=(const SimplePoolAllocator<T> &, const SimplePoolAllocator<U> &) {
    return false;
}

/**
 * 内存池封装类，管理不同大小的池
 */
class PoolManager {
  private:
    std::vector<std::unique_ptr<FixedBlockPool>> pools_;

  public:
    /**
     * 为特定大小获取或创建池
     */
    FixedBlockPool * get_pool(size_t block_size, size_t block_count = 1024) {
        // 简化实现：总是创建新池
        pools_.push_back(std::make_unique<FixedBlockPool>(block_size, block_count));
        return pools_.back().get();
    }

    void print_stats() {
        spdlog::info("\n=== PoolManager 统计 ===");
        spdlog::info("池数量: {}", pools_.size());
        for (size_t i = 0; i < pools_.size(); ++i) {
            spdlog::info("\n池 #{}:\n", i);
            pools_[i]->print_status();
        }
    }
};

} // namespace memory_pool

#endif // POOL_ALLOCATOR_H
