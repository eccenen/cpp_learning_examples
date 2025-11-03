/**
 * 固定块内存池示例程序
 */

#include <spdlog/spdlog.h>

#include <iostream>
#include <vector>

#include "../common/memory_pool_common.h"
#include "intermediate_fixed_block_pool.h"

using namespace memory_pool;

// 测试用的简单类
struct Point {
    double x, y;

    Point(double x_ = 0, double y_ = 0) : x(x_), y(y_) {
        spdlog::info("  Point({}, {}) 构造", x, y);
    }

    ~Point() { spdlog::info("  Point({}, {}) 析构", x, y); }
};

// 示例1：基本使用
void example_basic_usage() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例1：固定块池基本使用               ║");
    spdlog::info("╚════════════════════════════════════════╝");

    // 创建一个可以容纳10个整数的内存池
    FixedBlockPool pool(sizeof(int), 10);

    // 分配几个块
    int * p1 = static_cast<int *>(pool.allocate());
    int * p2 = static_cast<int *>(pool.allocate());
    int * p3 = static_cast<int *>(pool.allocate());

    if (p1 && p2 && p3) {
        *p1 = 100;
        *p2 = 200;
        *p3 = 300;

        spdlog::info("\n分配的值: {}, {}, {}", *p1, *p2, *p3);
    }

    pool.print_status();
    pool.visualize();

    // 释放
    pool.deallocate(p2);
    pool.deallocate(p1);

    pool.print_status();
    pool.visualize();

    // 再次分配（应该复用刚释放的）
    int * p4 = static_cast<int *>(pool.allocate());
    *p4      = 400;

    spdlog::info("\n重新分配的值: {}", *p4);

    pool.deallocate(p3);
    pool.deallocate(p4);

    pool.print_status();
}

// 示例2：对象构造和析构
void example_object_construction() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例2：在内存池中构造对象             ║");
    spdlog::info("╚════════════════════════════════════════╝");

    FixedBlockPool pool(sizeof(Point), 5);

    // 分配内存并使用placement new构造对象
    void *  mem1 = pool.allocate();
    Point * p1   = new (mem1) Point(1.0, 2.0);

    void *  mem2 = pool.allocate();
    Point * p2   = new (mem2) Point(3.0, 4.0);

    spdlog::info("\nPoint 1: ({}, {})", p1->x, p1->y);
    spdlog::info("Point 2: ({}, {})", p2->x, p2->y);

    pool.print_status();

    // 必须显式调用析构函数
    p1->~Point();
    pool.deallocate(p1);

    p2->~Point();
    pool.deallocate(p2);

    pool.print_status();
}

// 示例3：性能测试
void example_performance_test() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例3：性能对比                       ║");
    spdlog::info("╚════════════════════════════════════════╝");

    const size_t NUM_ALLOCATIONS = 100000;
    const size_t BLOCK_SIZE      = 32;

    // 测试内存池
    {
        FixedBlockPool pool(BLOCK_SIZE, NUM_ALLOCATIONS);
        Timer          timer;

        std::vector<void *> ptrs;
        ptrs.reserve(NUM_ALLOCATIONS);

        // 分配
        for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
            ptrs.push_back(pool.allocate());
        }

        double alloc_time = timer.elapsed_us();
        timer.reset();

        // 释放
        for (void * ptr : ptrs) {
            pool.deallocate(ptr);
        }

        double dealloc_time = timer.elapsed_us();

        spdlog::info("\n内存池性能:");
        spdlog::info("  分配 {} 次: {} μs", NUM_ALLOCATIONS, alloc_time);
        spdlog::info("  平均分配时间: {} μs", (alloc_time / NUM_ALLOCATIONS));
        spdlog::info("  释放时间: {} μs", dealloc_time);
        spdlog::info("  平均释放时间: {} μs", (dealloc_time / NUM_ALLOCATIONS));
    }

    // 测试标准new/delete
    {
        Timer timer;

        std::vector<void *> ptrs;
        ptrs.reserve(NUM_ALLOCATIONS);

        // 分配
        for (size_t i = 0; i < NUM_ALLOCATIONS; ++i) {
            ptrs.push_back(::operator new(BLOCK_SIZE));
        }

        double alloc_time = timer.elapsed_us();
        timer.reset();

        // 释放
        for (void * ptr : ptrs) {
            ::operator delete(ptr);
        }

        double dealloc_time = timer.elapsed_us();

        spdlog::info("\n标准 new/delete 性能:");
        spdlog::info("  分配 {} 次: {} μs", NUM_ALLOCATIONS, alloc_time);
        spdlog::info("  平均分配时间: {} μs", (alloc_time / NUM_ALLOCATIONS));
        spdlog::info("  释放时间: {} μs", dealloc_time);
        spdlog::info("  平均释放时间: {} μs", (dealloc_time / NUM_ALLOCATIONS));
    }
}

// 示例4：内存耗尽处理
void example_pool_exhaustion() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例4：内存池耗尽处理                 ║");
    spdlog::info("╚════════════════════════════════════════╝");

    FixedBlockPool pool(sizeof(int), 3); // 只有3个块

    std::vector<void *> ptrs;

    // 尝试分配4次
    for (int i = 0; i < 4; ++i) {
        void * ptr = pool.allocate();
        if (ptr) {
            spdlog::info("分配成功 #{}", i);
            ptrs.push_back(ptr);
        } else {
            spdlog::warn("分配失败 #{} - 内存池耗尽", i);
        }
    }

    pool.visualize();

    // 清理
    for (void * ptr : ptrs) {
        pool.deallocate(ptr);
    }
}

// 示例5：错误使用检测
void example_error_detection() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例5：错误使用检测                   ║");
    spdlog::info("╚════════════════════════════════════════╝");

    FixedBlockPool pool(sizeof(int), 5);

    int * p1 = static_cast<int *>(pool.allocate());
    *p1      = 42;

    // 错误1：释放不属于池的指针
    int external = 100;
    spdlog::info("\n尝试释放外部指针:");
    pool.deallocate(&external);

    // 错误2：释放nullptr
    spdlog::info("\n释放nullptr（安全）:");
    pool.deallocate(nullptr);

    // 正确释放
    pool.deallocate(p1);

    // 错误3：双重释放（会破坏空闲列表）
    spdlog::warn("\n注意：双重释放会破坏内存池结构");
    // pool.deallocate(p1);  // 危险！不要取消注释
}

int main() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 中级：固定块内存池教学示例             ║\n"
        "╚════════════════════════════════════════╝\n");

    example_basic_usage();
    example_object_construction();
    example_performance_test();
    example_pool_exhaustion();
    example_error_detection();

    spdlog::info("\n\n=== 学习要点总结 ===");
    spdlog::info("1. 固定块池适用于大量相同大小的对象分配");
    spdlog::info("2. 使用空闲列表实现O(1)的分配和释放");
    spdlog::info("3. placement new用于在预分配内存中构造对象");
    spdlog::info("4. 必须显式调用析构函数");
    spdlog::info("5. 性能通常比标准new/delete高很多");
    spdlog::info("6. 需要注意内存池容量限制");

    return 0;
}
