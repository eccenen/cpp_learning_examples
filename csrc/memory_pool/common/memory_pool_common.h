#ifndef MEMORY_POOL_COMMON_H
#define MEMORY_POOL_COMMON_H

#include "common.h"

namespace memory_pool {

// 内存统计结构
struct MemoryStats {
    std::atomic<size_t> total_allocated{ 0 };
    std::atomic<size_t> total_freed{ 0 };
    std::atomic<size_t> current_usage{ 0 };
    std::atomic<size_t> peak_usage{ 0 };
    std::atomic<size_t> allocation_count{ 0 };
    std::atomic<size_t> deallocation_count{ 0 };

    void recordAllocation(size_t size) {
        total_allocated += size;
        current_usage += size;
        allocation_count++;

        // 更新峰值使用
        size_t current = current_usage.load();
        size_t peak    = peak_usage.load();
        while (current > peak && !peak_usage.compare_exchange_weak(peak, current)) {
        }
    }

    void recordDeallocation(size_t size) {
        total_freed += size;
        current_usage -= size;
        deallocation_count++;
    }

    // 兼容旧命名
    void record_allocation(size_t size) { recordAllocation(size); }

    void record_deallocation(size_t size) { recordDeallocation(size); }

    void reset() {
        total_allocated    = 0;
        total_freed        = 0;
        current_usage      = 0;
        peak_usage         = 0;
        allocation_count   = 0;
        deallocation_count = 0;
    }

    void show() const {
        spdlog::info("=== 内存统计 ===");
        spdlog::info("总分配: {} bytes ({} 次)", total_allocated.load(), allocation_count.load());
        spdlog::info("总释放: {} bytes ({} 次)", total_freed.load(), deallocation_count.load());
        spdlog::info("当前使用: {} bytes", current_usage.load());
        spdlog::info("峰值使用: {} bytes", peak_usage.load());
        spdlog::info("泄漏检测: {} bytes", total_allocated.load() - total_freed.load());
    }
};

// 兼容别名：老代码中可能使用 PoolStats 命名
using PoolStats = MemoryStats;

// 性能计时器
class Timer {
  public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsedMs() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start_).count();
    }

    double elapsedUs() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::micro>(end - start_).count();
    }

    void reset() { start_ = std::chrono::high_resolution_clock::now(); }

    // 兼容旧命名
    double elapsed_ms() const { return elapsedMs(); }

    double elapsed_us() const { return elapsedUs(); }

    void reset_timer() { reset(); }

  private:
    std::chrono::high_resolution_clock::time_point start_;
};

// RAII 内存保护
template <typename T> class MemoryGuard {
  public:
    explicit MemoryGuard(T * ptr) : ptr_(ptr) {}

    ~MemoryGuard() { delete ptr_; }

    MemoryGuard(const MemoryGuard &)             = delete;
    MemoryGuard & operator=(const MemoryGuard &) = delete;

    T * data() const { return ptr_; }

    T * release() {
        T * tmp = ptr_;
        ptr_    = nullptr;
        return tmp;
    }

  private:
    T * ptr_;
};

// 内存池配置
struct PoolConfig {
    size_t block_size       = 32; // 块大小
    size_t block_count      = 1024; // 块数量
    size_t alignment        = alignof(std::max_align_t); // 对齐大小
    bool   enable_stats     = true; // 启用统计
    bool   enable_threading = false; // 启用线程安全
};

// 内存对齐工具
inline size_t alignUp(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

inline bool isAligned(void * ptr, size_t alignment) {
    return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
}

// 检查是否是2的幂
inline bool isPowerOfTwo(size_t x) {
    return x && !(x & (x - 1));
}

// 兼容老命名（snake_case）
inline size_t align_up(size_t size, size_t alignment) {
    return alignUp(size, alignment);
}

inline bool is_power_of_two(size_t x) {
    return isPowerOfTwo(x);
}

} // namespace memory_pool

#endif // MEMORY_POOL_COMMON_H
