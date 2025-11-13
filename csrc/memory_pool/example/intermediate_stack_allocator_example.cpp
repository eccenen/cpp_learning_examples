/**
 * 栈式分配器示例程序
 */

#include "../common/memory_pool_common.h"
#include "common.h"
#include "intermediate_stack_allocator.h"

using namespace memory_pool;

// 示例1：基本使用
void example_basic_usage() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 示例1：栈式分配器基本使用             ║\n"
        "╚════════════════════════════════════════╝\n");

    StackAllocator stack(1024);

    // 分配一些内存
    int * arr1 = static_cast<int *>(stack.allocate(10 * sizeof(int)));
    for (int i = 0; i < 10; ++i) {
        arr1[i] = i;
    }

    double * arr2 = static_cast<double *>(stack.allocate(5 * sizeof(double)));
    for (int i = 0; i < 5; ++i) {
        arr2[i] = i * 1.5;
    }

    stack.printStatus();
    stack.visualize();

    spdlog::info("arr1前3个元素: {} {} {}", arr1[0], arr1[1], arr1[2]);
    spdlog::info("arr2前3个元素: {} {} {}", arr2[0], arr2[1], arr2[2]);

    // 清空
    stack.clear();
    stack.printStatus();
}

// 示例2：标记和恢复
void example_markers() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 示例2：标记和恢复                     ║\n"
        "╚════════════════════════════════════════╝\n");

    StackAllocator stack(1024);

    // 第一次分配
    int * p1 = static_cast<int *>(stack.allocate(100));
    *p1      = 100;
    spdlog::info("分配1: {} bytes", stack.used());

    // 保存标记
    auto marker = stack.getMarker();
    spdlog::info("保存标记");

    // 第二次分配
    int * p2 = static_cast<int *>(stack.allocate(200));
    *p2      = 200;
    spdlog::info("分配2: {} bytes", stack.used());

    // 第三次分配
    int * p3 = static_cast<int *>(stack.allocate(300));
    *p3      = 300;
    spdlog::info("分配3: {} bytes", stack.used());

    stack.visualize();

    // 恢复到标记点
    spdlog::info("\n恢复到标记点...");
    stack.freeToMarker(marker);
    spdlog::info("恢复后: {} bytes", stack.used());

    // p1仍然有效，但p2和p3已失效
    spdlog::info("p1的值: {} (仍然有效)", *p1);

    stack.visualize();
}

// 示例3：RAII作用域
void example_scoped_allocation() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 示例3：RAII作用域自动管理             ║\n"
        "╚════════════════════════════════════════╝\n");

    StackAllocator stack(1024);

    int * p1 = static_cast<int *>(stack.allocate(100));
    *p1      = 100;
    spdlog::info("外层分配: {} bytes", stack.used());

    {
        StackAllocatorScope scope(stack); // 自动push标记

        int * p2 = static_cast<int *>(stack.allocate(200));
        *p2      = 200;
        spdlog::info("内层分配: {} bytes", stack.used());

        {
            StackAllocatorScope inner_scope(stack);

            int * p3 = static_cast<int *>(stack.allocate(300));
            *p3      = 300;
            spdlog::info("最内层分配: {} bytes", stack.used());

            stack.visualize();
        } // 自动pop，释放p3

        spdlog::info("最内层作用域结束: {} bytes", stack.used());
    } // 自动pop，释放p2

    spdlog::info("内层作用域结束: {} bytes", stack.used());
    spdlog::info("p1仍然有效: {}", *p1);
}

// 示例4：临时对象分配
void process_data(StackAllocator & stack) {
    StackAllocatorScope scope(stack); // 函数结束时自动清理

    // 分配临时缓冲区
    constexpr size_t BUFFER_SIZE = 1000;
    char *           buffer      = static_cast<char *>(stack.allocate(BUFFER_SIZE));

    // 使用buffer进行处理...
    std::snprintf(buffer, BUFFER_SIZE, "Processing data...");
    spdlog::info("  {}", buffer);

    // 不需要手动释放，函数返回时自动清理
}

void example_temporary_allocations() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 示例4：临时对象快速分配               ║\n"
        "╚════════════════════════════════════════╝\n");

    StackAllocator stack(4096);

    spdlog::info("初始状态: {} bytes", stack.used());

    for (int i = 0; i < 3; ++i) {
        spdlog::info("\n处理第 {} 次:", i);
        process_data(stack);
        spdlog::info("处理后: {} bytes", stack.used());
    }

    spdlog::info("\n所有处理完成，内存自动恢复");
}

// 示例5：性能测试
void example_performance() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 示例5：性能对比                       ║\n"
        "╚════════════════════════════════════════╝\n");

    const size_t NUM_ITERATIONS = 100000;
    const size_t ALLOC_SIZE     = 64;

    // 测试栈分配器
    {
        StackAllocator stack(NUM_ITERATIONS * ALLOC_SIZE);
        Timer          timer;

        for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
            void * ptr = stack.allocate(ALLOC_SIZE);
            (void) ptr; // 避免优化掉
        }

        double elapsed = timer.elapsedUs();

        spdlog::info("\n栈分配器:");
        spdlog::info("  {} 次分配", NUM_ITERATIONS);
        spdlog::info("  总时间: {} μs", elapsed);
        spdlog::info("  平均: {} μs/次", (elapsed / NUM_ITERATIONS));
    }

    // 测试标准new
    {
        Timer               timer;
        std::vector<void *> ptrs;
        ptrs.reserve(NUM_ITERATIONS);

        for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
            ptrs.push_back(::operator new(ALLOC_SIZE));
        }

        double alloc_time = timer.elapsedUs();
        timer.reset();

        for (void * ptr : ptrs) {
            ::operator delete(ptr);
        }

        double dealloc_time = timer.elapsedUs();

        spdlog::info("\n标准 new/delete:");
        spdlog::info("  {} 次分配", NUM_ITERATIONS);
        spdlog::info("  分配时间: {} μs", alloc_time);
        spdlog::info("  平均分配: {} μs/次", (alloc_time / NUM_ITERATIONS));
        spdlog::info("  释放时间: {} μs", dealloc_time);
    }
}

// 示例6：对齐分配
void example_aligned_allocation() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 示例6：对齐内存分配                   ║\n"
        "╚════════════════════════════════════════╝\n");

    StackAllocator stack(1024);

    // 分配未对齐的内存
    void * p1 = stack.allocate(1);
    spdlog::info("p1地址: {:p} (对齐到 {})", p1, (reinterpret_cast<uintptr_t>(p1) % 16));

    // 分配16字节对齐的内存
    void * p2 = stack.allocate(64, 16);
    spdlog::info("p2地址: {:p} (对齐到 {})", p2, (reinterpret_cast<uintptr_t>(p2) % 16));

    // 分配64字节对齐的内存（SIMD）
    void * p3 = stack.allocate(128, 64);
    spdlog::info("p3地址: {:p} (对齐到 {})", p3, (reinterpret_cast<uintptr_t>(p3) % 64));

    stack.printStatus();
}

int main() {
    spdlog::info(
        "\n╔════════════════════════════════════════╗\n"
        "║ 中级：栈式分配器教学示例               ║\n"
        "╚════════════════════════════════════════╝\n");

    example_basic_usage();
    example_markers();
    example_scoped_allocation();
    example_temporary_allocations();
    example_performance();
    example_aligned_allocation();

    spdlog::info("\n\n=== 学习要点总结 ===");
    spdlog::info("1. 栈分配器提供极快的顺序分配（只需移动指针）");
    spdlog::info("2. 适用于生命周期明确的临时对象");
    spdlog::info("3. 使用标记和恢复机制批量释放内存");
    spdlog::info("4. RAII作用域保护简化了内存管理");
    spdlog::info("5. 性能远超标准new/delete");
    spdlog::info("6. 限制：只能按LIFO顺序释放");
    spdlog::info("7. 常用场景：帧分配器、临时计算缓冲区");

    return 0;
}
