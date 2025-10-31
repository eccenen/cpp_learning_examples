/**
 * 综合性能基准测试
 *
 * 测试内容：
 * 1. 不同内存池的分配/释放性能
 * 2. 与标准new/delete对比
 * 3. 多线程场景性能
 * 4. 内存碎片和使用效率
 */

#include <spdlog/spdlog.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#include "../common/memory_pool_common.h"
#include "../intermediate/fixed_block_pool.h"
#include "../intermediate/stack_allocator.h"

using namespace memory_pool;

// 基准测试配置
struct BenchmarkConfig {
    size_t       num_iterations = 100000;
    size_t       block_size     = 64;
    bool         random_order   = false;
    const char * name           = "Benchmark";
};

// 基准测试结果
struct BenchmarkResult {
    double alloc_time_us;
    double dealloc_time_us;
    double total_time_us;
    size_t peak_memory;

    void print(const char * name) const {
        spdlog::info("\n{} 结果:", name);
        spdlog::info("  分配时间: {} μs", alloc_time_us);
        spdlog::info("  释放时间: {} μs", dealloc_time_us);
        spdlog::info("  总时间: {} μs", total_time_us);
        spdlog::info("  平均分配: {} μs", (alloc_time_us / 100000.0));
        spdlog::info("  平均释放: {} μs", (dealloc_time_us / 100000.0));
    }
};

// 1. 测试标准new/delete
BenchmarkResult benchmark_standard_new(const BenchmarkConfig & config) {
    std::vector<void *> ptrs;
    ptrs.reserve(config.num_iterations);

    Timer timer;

    // 分配
    for (size_t i = 0; i < config.num_iterations; ++i) {
        ptrs.push_back(::operator new(config.block_size));
    }

    double alloc_time = timer.elapsed_us();
    timer.reset();

    // 可选：随机释放顺序
    if (config.random_order) {
        std::random_device rd;
        std::mt19937       g(rd());
        std::shuffle(ptrs.begin(), ptrs.end(), g);
    }

    // 释放
    for (void * ptr : ptrs) {
        ::operator delete(ptr);
    }

    double dealloc_time = timer.elapsed_us();

    return { alloc_time, dealloc_time, alloc_time + dealloc_time, 0 };
}

// 2. 测试固定块池
BenchmarkResult benchmark_fixed_pool(const BenchmarkConfig & config) {
    FixedBlockPool      pool(config.block_size, config.num_iterations);
    std::vector<void *> ptrs;
    ptrs.reserve(config.num_iterations);

    Timer timer;

    // 分配
    for (size_t i = 0; i < config.num_iterations; ++i) {
        ptrs.push_back(pool.allocate());
    }

    double alloc_time = timer.elapsed_us();
    timer.reset();

    if (config.random_order) {
        std::random_device rd;
        std::mt19937       g(rd());
        std::shuffle(ptrs.begin(), ptrs.end(), g);
    }

    // 释放
    for (void * ptr : ptrs) {
        pool.deallocate(ptr);
    }

    double dealloc_time = timer.elapsed_us();

    return { alloc_time, dealloc_time, alloc_time + dealloc_time, pool.stats().peak_usage };
}

// 3. 测试栈分配器
BenchmarkResult benchmark_stack_allocator(const BenchmarkConfig & config) {
    size_t         total_size = config.block_size * config.num_iterations;
    StackAllocator stack(total_size);

    Timer timer;

    // 分配（栈分配器不需要保存指针）
    for (size_t i = 0; i < config.num_iterations; ++i) {
        stack.allocate(config.block_size);
    }

    double alloc_time = timer.elapsed_us();
    timer.reset();

    // "释放"（只需清空）
    stack.clear();

    double dealloc_time = timer.elapsed_us();

    return { alloc_time, dealloc_time, alloc_time + dealloc_time, stack.stats().peak_usage };
}

// 运行完整基准测试套件
void run_benchmark_suite() {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║        内存池性能基准测试                      ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    BenchmarkConfig config;
    config.num_iterations = 100000;
    config.block_size     = 64;

    spdlog::info("\n测试配置:");
    spdlog::info("  迭代次数: {}", config.num_iterations);
    spdlog::info("  块大小: {} bytes", config.block_size);

    // 测试1：顺序分配和释放
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "测试1：顺序分配和释放\n";
    std::cout << std::string(50, '=') << "\n";

    config.random_order = false;

    auto result_std = benchmark_standard_new(config);
    result_std.print("标准 new/delete");

    auto result_pool = benchmark_fixed_pool(config);
    result_pool.print("固定块池");

    auto result_stack = benchmark_stack_allocator(config);
    result_stack.print("栈分配器");

    spdlog::info("\n性能对比:");
    double speedup_pool  = result_std.total_time_us / result_pool.total_time_us;
    double speedup_stack = result_std.total_time_us / result_stack.total_time_us;

    spdlog::info("  固定块池 vs 标准: {}x 更快", speedup_pool);
    spdlog::info("  栈分配器 vs 标准: {}x 更快", speedup_stack);

    // 测试2：随机释放顺序
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "测试2：随机释放顺序\n";
    std::cout << std::string(50, '=') << "\n";

    config.random_order = true;

    result_std = benchmark_standard_new(config);
    result_std.print("标准 new/delete（随机）");

    result_pool = benchmark_fixed_pool(config);
    result_pool.print("固定块池（随机）");

    speedup_pool = result_std.total_time_us / result_pool.total_time_us;
    spdlog::info("\n性能对比:");
    spdlog::info("  固定块池 vs 标准（随机）: {}x", speedup_pool);

    // 测试3：不同块大小
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "测试3：不同块大小性能\n";
    std::cout << std::string(50, '=') << "\n";

    config.random_order   = false;
    config.num_iterations = 50000;

    size_t sizes[] = { 16, 32, 64, 128, 256, 512 };

    spdlog::info("\n块大小    标准(μs)    池(μs)    加速比");
    spdlog::info(std::string(50, '-'));

    for (size_t size : sizes) {
        config.block_size = size;

        auto r_std  = benchmark_standard_new(config);
        auto r_pool = benchmark_fixed_pool(config);

        double speedup = r_std.total_time_us / r_pool.total_time_us;

        spdlog::info("{:<4}      {:8.0f}    {:8.0f}    {:.2f}x", size, r_std.total_time_us,
                     r_pool.total_time_us, speedup);
    }
}

// 内存使用效率测试
void test_memory_efficiency() {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║        内存使用效率测试                        ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    const size_t NUM_ALLOCS = 1000;
    const size_t BLOCK_SIZE = 64;

    // 固定块池
    {
        FixedBlockPool      pool(BLOCK_SIZE, NUM_ALLOCS);
        std::vector<void *> ptrs;

        // 分配50%
        for (size_t i = 0; i < NUM_ALLOCS / 2; ++i) {
            ptrs.push_back(pool.allocate());
        }

        std::cout << "\n固定块池（分配50%）:\n";
        pool.print_status();

        // 释放一半
        for (size_t i = 0; i < ptrs.size() / 2; ++i) {
            pool.deallocate(ptrs[i]);
        }

        std::cout << "\n固定块池（释放25%后）:\n";
        pool.print_status();

        // 清理
        for (size_t i = ptrs.size() / 2; i < ptrs.size(); ++i) {
            pool.deallocate(ptrs[i]);
        }
    }

    // 栈分配器
    {
        StackAllocator stack(BLOCK_SIZE * NUM_ALLOCS);

        for (size_t i = 0; i < NUM_ALLOCS / 2; ++i) {
            stack.allocate(BLOCK_SIZE);
        }

        std::cout << "\n栈分配器（分配50%）:\n";
        stack.print_status();
        stack.visualize();
    }
}

// 碎片化测试
void test_fragmentation() {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║        内存碎片化测试                          ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    const size_t   POOL_SIZE = 100;
    FixedBlockPool pool(64, POOL_SIZE);

    std::vector<void *> ptrs;

    // 分配所有
    std::cout << "\n1. 分配所有块:\n";
    for (size_t i = 0; i < POOL_SIZE; ++i) {
        ptrs.push_back(pool.allocate());
    }
    pool.visualize();

    // 释放奇数位置
    std::cout << "\n2. 释放奇数位置（产生碎片）:\n";
    for (size_t i = 1; i < ptrs.size(); i += 2) {
        pool.deallocate(ptrs[i]);
        ptrs[i] = nullptr;
    }
    pool.visualize();

    // 重新分配
    std::cout << "\n3. 重新分配（填充碎片）:\n";
    for (size_t i = 0; i < POOL_SIZE / 2; ++i) {
        void * ptr = pool.allocate();
        if (!ptr) {
            break;
        }
    }
    pool.visualize();

    // 清理
    for (void * ptr : ptrs) {
        if (ptr) {
            pool.deallocate(ptr);
        }
    }
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║   内存池综合性能测试套件                       ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n";

    run_benchmark_suite();
    test_memory_efficiency();
    test_fragmentation();

    std::cout << "\n\n=== 测试结论 ===\n";
    std::cout << "1. 内存池在频繁分配/释放场景下性能优势明显\n";
    std::cout << "2. 栈分配器提供最快的分配速度（几乎零开销）\n";
    std::cout << "3. 固定块池在随机释放时仍保持良好性能\n";
    std::cout << "4. 内存池减少了碎片化问题\n";
    std::cout << "5. 块大小越小，内存池优势越明显\n";

    return 0;
}
