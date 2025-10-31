/**
 * 中级教程2：栈式分配器（Stack Allocator）
 *
 * 学习目标：
 * 1. 理解栈式内存分配模式
 * 2. 实现极快的顺序分配
 * 3. 理解"后进先出"的释放约束
 * 4. 适用于临时对象的快速分配
 *
 * 内存布局：
 * ┌─────────────────────────────────┐
 * │░░░░░░░░░░░░░░░░░░               │
 * └─────────────────────────────────┘
 *  ↑              ↑                ↑
 * start        current            end
 * (used)                      (free)
 */

#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H

#include <cstdlib>

#include "../common/memory_pool_common.h"
#include "common.h"

namespace memory_pool {

/**
 * 栈式分配器
 * 快速顺序分配，但只能按照LIFO顺序释放
 */
class StackAllocator {
  private:
    char *      buffer_; // 内存缓冲区
    size_t      capacity_; // 总容量
    size_t      offset_; // 当前分配偏移
    MemoryStats stats_; // 统计信息

    // 用于标记和恢复的结构
    struct Marker {
        size_t offset;
    };

    std::vector<Marker> markers_; // 标记栈

  public:
    /**
     * 构造栈式分配器
     * @param capacity 缓冲区大小（字节）
     */
    explicit StackAllocator(size_t capacity) : capacity_(capacity), offset_(0) {
        buffer_ = static_cast<char *>(::operator new(capacity_));

        spdlog::info("[StackAllocator] 初始化:");
        spdlog::info("  容量: {} bytes", capacity_);
        spdlog::info("  地址: {}", static_cast<void *>(buffer_));
    }

    ~StackAllocator() {
        if (offset_ > 0) {
            spdlog::warn("[警告] 栈分配器销毁时还有 {} bytes未释放", offset_);
        }

        ::operator delete(buffer_);
        spdlog::info("[StackAllocator] 销毁");
    }

    // 禁止拷贝
    StackAllocator(const StackAllocator &)             = delete;
    StackAllocator & operator=(const StackAllocator &) = delete;

    /**
     * 分配内存（对齐版本）
     * @param size 请求的字节数
     * @param alignment 对齐要求
     * @return 指向分配内存的指针，失败返回nullptr
     */
    void * allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        // 计算对齐后的偏移
        size_t current_addr = reinterpret_cast<uintptr_t>(buffer_) + offset_;
        size_t aligned_addr = alignUp(current_addr, alignment);
        size_t padding      = aligned_addr - current_addr;

        size_t new_offset = offset_ + padding + size;

        // 检查是否有足够空间
        if (new_offset > capacity_) {
            spdlog::error("[错误] StackAllocator 空间不足: 需要 {} bytes, 容量 {} bytes",
                          new_offset, capacity_);
            return nullptr;
        }

        void * ptr = buffer_ + offset_ + padding;
        offset_    = new_offset;

        stats_.recordAllocation(size);

        return ptr;
    }

    /**
     * 释放到指定标记点
     * 注意：不能单独释放某个分配，只能恢复到之前的标记
     */
    void freeToMarker(Marker marker) {
        if (marker.offset > offset_) {
            spdlog::error("[错误] 无效的标记");
            return;
        }

        size_t freed = offset_ - marker.offset;
        offset_      = marker.offset;

        stats_.recordDeallocation(freed);
    }

    /**
     * 获取当前标记（用于后续恢复）
     */
    Marker getMarker() const { return Marker{ offset_ }; }

    /**
     * 清空分配器（释放所有）
     */
    void clear() {
        size_t freed = offset_;
        offset_      = 0;

        if (freed > 0) {
            stats_.recordDeallocation(freed);
        }

        spdlog::info("[StackAllocator] 清空，释放 {} bytes", freed);
    }

    /**
     * 推入标记（保存当前状态）
     */
    void pushMarker() { markers_.push_back(getMarker()); }

    /**
     * 弹出标记（恢复到保存的状态）
     */
    void pop_marker() {
        if (markers_.empty()) {
            spdlog::error("[错误] 标记栈为空");
            return;
        }

        Marker marker = markers_.back();
        markers_.pop_back();
        freeToMarker(marker);
    }

    /**
     * 获取当前使用量
     */
    size_t used() const { return offset_; }

    /**
     * 获取剩余空间
     */
    size_t available() const { return capacity_ - offset_; }

    /**
     * 获取统计信息
     */
    const MemoryStats & stats() const { return stats_; }

    /**
     * 打印状态
     */
    void printStatus() const {
        spdlog::info("\n=== StackAllocator 状态 ===");
        spdlog::info("容量: {} bytes", capacity_);
        spdlog::info("已用: {} bytes", offset_);
        spdlog::info("可用: {} bytes", available());
        spdlog::info("使用率: {}%", (100.0 * offset_ / capacity_));
        spdlog::info("标记数: {}", markers_.size());

        stats_.show();
    }

    /**
     * 可视化内存使用
     */
    void visualize() const {
        spdlog::info("\n=== 栈分配器可视化 ===");

        const size_t bar_width  = 50;
        size_t       used_width = (bar_width * offset_) / capacity_;

        std::string bar;
        bar.reserve(bar_width);
        for (size_t i = 0; i < bar_width; ++i) {
            bar.push_back(i < used_width ? '#' : ' ');
        }
        spdlog::info("[{}] {}%", bar, (100.0 * offset_ / capacity_));
        spdlog::info("已用: {} / {} bytes", offset_, capacity_);
    }
};

/**
 * RAII风格的栈分配器作用域保护
 * 自动管理标记的push和pop
 */
class StackAllocatorScope {
  private:
    StackAllocator & allocator_;

  public:
    explicit StackAllocatorScope(StackAllocator & alloc) : allocator_(alloc) {
        allocator_.pushMarker();
    }

    ~StackAllocatorScope() { allocator_.pop_marker(); }

    // 禁止拷贝和移动
    StackAllocatorScope(const StackAllocatorScope &)             = delete;
    StackAllocatorScope & operator=(const StackAllocatorScope &) = delete;
};

} // namespace memory_pool

#endif // STACK_ALLOCATOR_H
