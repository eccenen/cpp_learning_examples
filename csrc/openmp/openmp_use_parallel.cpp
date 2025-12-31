/**
 * @file openmp_use_parallel.cpp
 * @brief OpenMP 设置并行区域线程数的四种方式详解
 *
 * 设置线程数的四种方式：
 * 1. 默认方式 - 使用系统默认线程数
 * 2. 库函数方式 - omp_set_num_threads()
 * 3. 指令方式 - num_threads(n)
 * 4. 环境变量 - OMP_NUM_THREADS
 *
 * 优先级（从高到低）：
 * num_threads() > omp_set_num_threads() > OMP_NUM_THREADS > 系统默认
 */

#include <omp.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>

// ============================================================================
// 辅助函数：打印线程信息
// ============================================================================
void print_thread_info(const char * desc) {
#pragma omp parallel
    {
#pragma omp single
        { std::cout << desc << " - 并行区域线程数: " << omp_get_num_threads() << "\n"; }
    }
}

// ============================================================================
// 1. 默认方式 - 系统自动决定线程数
// ============================================================================
/**
 * 特点：静态设置
 * - 不显式指定线程数，由 OpenMP 运行时决定
 * - 通常等于 CPU 核心数（逻辑处理器数）
 * - 可通过 OMP_NUM_THREADS 环境变量影响
 */
void demo_default_threads() {
    std::cout << "\n=== 1. 默认方式 ===\n";
    std::cout << "系统最大可用线程数: " << omp_get_max_threads() << "\n";

#pragma omp parallel
    {
#pragma omp single
        { std::cout << "默认并行区域线程数: " << omp_get_num_threads() << "\n"; }
    }

    std::cout << "特点: 静态设置，编译时或启动时确定\n";
}

// ============================================================================
// 2. 库函数方式 - omp_set_num_threads()
// ============================================================================
/**
 * 特点：动态设置
 * - 运行时可修改线程数
 * - 影响后续所有并行区域（除非被 num_threads 覆盖）
 * - 作用域：从调用点开始的整个程序或线程
 */
void demo_library_function() {
    std::cout << "\n=== 2. 库函数方式 (动态设置) ===\n";

    // 设置为 2 个线程
    omp_set_num_threads(2);
    print_thread_info("设置 omp_set_num_threads(2)");

    // 再次设置为 4 个线程
    omp_set_num_threads(4);
    print_thread_info("设置 omp_set_num_threads(4)");

    // 设置为 1 个线程（串行模式）
    omp_set_num_threads(1);
    print_thread_info("设置 omp_set_num_threads(1)");

    std::cout << "特点: 动态设置，运行时可随时修改\n";
}

// ============================================================================
// 3. 指令方式 - num_threads(n)
// ============================================================================
/**
 * 特点：局部动态设置
 * - 只影响紧邻的并行区域
 * - 优先级最高，覆盖其他设置
 * - 精确控制特定并行区域的线程数
 */
void demo_directive_clause() {
    std::cout << "\n=== 3. 指令方式 (局部动态设置) ===\n";

    omp_set_num_threads(8); // 全局设置为 8
    std::cout << "全局设置: omp_set_num_threads(8)\n";

// 使用 num_threads 覆盖全局设置
#pragma omp parallel num_threads(3)
    {
#pragma omp single
        { std::cout << "num_threads(3) 并行区域: " << omp_get_num_threads() << " 线程\n"; }
    }

    // 下一个并行区域恢复全局设置
    print_thread_info("下一个并行区域（无 num_threads）");

    std::cout << "特点: 局部动态设置，只影响当前并行区域\n";
}

// ============================================================================
// 4. 环境变量方式 - OMP_NUM_THREADS
// ============================================================================
/**
 * 特点：静态设置
 * - 程序启动前设置：export OMP_NUM_THREADS=6
 * - 优先级低于库函数和指令
 * - 适合在不修改代码的情况下调整性能
 */
void demo_environment_variable() {
    std::cout << "\n=== 4. 环境变量方式 (静态设置) ===\n";

    const char * env_threads = std::getenv("OMP_NUM_THREADS");
    if (env_threads) {
        std::cout << "环境变量 OMP_NUM_THREADS = " << env_threads << "\n";
    } else {
        std::cout << "未设置 OMP_NUM_THREADS 环境变量\n";
    }

    std::cout << "当前最大线程数: " << omp_get_max_threads() << "\n";
    std::cout << "特点: 静态设置，程序启动前设置\n";
    std::cout << "设置方法: export OMP_NUM_THREADS=6\n";
}

// ============================================================================
// 优先级演示
// ============================================================================
void demo_priority() {
    std::cout << "\n=== 优先级演示 ===\n";
    std::cout << "优先级: num_threads() > omp_set_num_threads() > " << "OMP_NUM_THREADS > 默认\n\n";

    // 设置全局线程数
    omp_set_num_threads(10);

    std::cout << "1. 设置 omp_set_num_threads(10)\n";
    print_thread_info("   无 num_threads 子句");

    std::cout << "2. 使用 num_threads(3) 覆盖\n";
#pragma omp parallel num_threads(3)
    {
#pragma omp single
        { std::cout << "   有 num_threads(3): " << omp_get_num_threads() << " 线程\n"; }
    }
}

// ============================================================================
// 模式详解：嵌套并行
// ============================================================================
/**
 * 嵌套并行模式：并行区域内再创建并行区域
 * - 需要启用：omp_set_nested(1) 或 OMP_NESTED=true
 * - 可为每层设置不同线程数
 */
void demo_nested_parallelism() {
    std::cout << "\n=== 嵌套并行模式 ===\n";

    // 启用嵌套并行（OpenMP 3.0+）
    omp_set_nested(1);
    // 或使用新接口（OpenMP 5.0+）
    // omp_set_max_active_levels(2);

    std::cout << "嵌套并行已启用: " << (omp_get_nested() ? "是" : "否") << "\n";

    omp_set_num_threads(2); // 外层 2 个线程

#pragma omp parallel
    {
        int outer_tid   = omp_get_thread_num();
        int outer_total = omp_get_num_threads();

#pragma omp critical
        { std::cout << "外层线程 " << outer_tid << "/" << outer_total << "\n"; }

// 内层并行区域
#pragma omp parallel num_threads(3)
        {
            int inner_tid   = omp_get_thread_num();
            int inner_total = omp_get_num_threads();

#pragma omp critical
            {
                std::cout << "  └─ 内层线程 " << inner_tid << "/" << inner_total << " (外层线程 "
                          << outer_tid << ")\n";
            }
        }
    }

    std::cout << "特点: 多层并行，每层可独立设置线程数\n";
    std::cout << "注意: 总线程数 = 外层线程数 × 内层线程数\n";

    omp_set_nested(0); // 恢复默认
}

// ============================================================================
// 模式详解：动态调整
// ============================================================================
/**
 * 动态调整模式：运行时根据系统负载调整线程数
 * - 启用：omp_set_dynamic(1) 或 OMP_DYNAMIC=true
 * - OpenMP 可减少实际线程数（但不会超过指定数）
 */
void demo_dynamic_adjustment() {
    std::cout << "\n=== 动态调整模式 ===\n";

    std::cout << "当前动态调整状态: " << (omp_get_dynamic() ? "启用" : "禁用") << "\n";

    // 启用动态调整
    omp_set_dynamic(1);
    std::cout << "启用动态调整\n";

    omp_set_num_threads(8);
    std::cout << "请求 8 个线程，实际可能更少\n";

#pragma omp parallel
    {
#pragma omp single
        { std::cout << "实际获得线程数: " << omp_get_num_threads() << "\n"; }
    }

    std::cout << "特点: OpenMP 根据系统负载自动调整\n";
    std::cout << "注意: 实际线程数 ≤ 请求线程数\n";

    omp_set_dynamic(0); // 恢复默认
}

// ============================================================================
// 实用示例：不同任务使用不同线程数
// ============================================================================
void demo_practical_example() {
    std::cout << "\n=== 实用示例：不同任务不同线程数 ===\n";

    // 任务 1：CPU 密集型，使用所有核心
    omp_set_num_threads(omp_get_max_threads());
    std::cout << "CPU 密集型任务（矩阵运算）\n";
    print_thread_info("  使用全部线程");

    // 任务 2：I/O 密集型，线程数适中
    omp_set_num_threads(4);
    std::cout << "I/O 密集型任务（文件处理）\n";
    print_thread_info("  使用 4 个线程");

    // 任务 3：内存密集型，线程数较少避免竞争
    omp_set_num_threads(2);
    std::cout << "内存密集型任务（大数据访问）\n";
    print_thread_info("  使用 2 个线程");
}

// ============================================================================
// 三种模式总结
// ============================================================================
void print_mode_summary() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "三种模式详解总结\n";
    std::cout << std::string(60, '=') << "\n";

    std::cout << "\n【静态设置】\n";
    std::cout << "  定义: 程序启动前或编译时确定线程数\n";
    std::cout << "  方式: 默认方式、环境变量 OMP_NUM_THREADS\n";
    std::cout << "  特点: 整个程序运行期间不变\n";
    std::cout << "  适用: 负载稳定、线程数固定的场景\n";

    std::cout << "\n【动态设置】\n";
    std::cout << "  定义: 运行时可修改线程数\n";
    std::cout << "  方式: omp_set_num_threads()、num_threads()\n";
    std::cout << "  特点: 灵活调整，适应不同并行区域需求\n";
    std::cout << "  适用: 不同阶段需要不同线程数的场景\n";

    std::cout << "\n【嵌套并行】\n";
    std::cout << "  定义: 并行区域内再创建并行区域\n";
    std::cout << "  启用: omp_set_nested(1) 或 OMP_NESTED=true\n";
    std::cout << "  特点: 多层并行，每层独立设置线程数\n";
    std::cout << "  适用: 复杂的多层次并行算法\n";
    std::cout << "  注意: 总线程数激增，需谨慎使用\n";

    std::cout << "\n【动态调整模式】\n";
    std::cout << "  定义: OpenMP 根据系统负载自动调整线程数\n";
    std::cout << "  启用: omp_set_dynamic(1) 或 OMP_DYNAMIC=true\n";
    std::cout << "  特点: 自适应，但实际线程数 ≤ 请求数\n";
    std::cout << "  适用: 系统负载不确定的共享环境\n";
}

// ============================================================================
// IF 子句详解 - 条件性并行
// ============================================================================
/**
 * OpenMP if 子句：根据条件决定是否并行执行
 *
 * 语法: #pragma omp parallel if(condition)
 *
 * 工作原理:
 *   - condition 为 true  → 多线程并行执行
 *   - condition 为 false → 单线程串行执行（主线程）
 *
 * 核心价值:
 *   避免小规模任务的并行开销（线程创建/销毁/同步的成本）
 */
void demo_if_clause() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "IF 子句详解 - 条件性并行\n";
    std::cout << std::string(60, '=') << "\n";

    omp_set_num_threads(4);

    // ========================================================================
    // 场景 1: 基础用法 - 数据量阈值判断
    // ========================================================================
    std::cout << "\n【场景 1】数据量阈值判断（最常用）\n";
    std::cout << "原理: 数据量小时，并行开销 > 并行收益\n\n";

    constexpr int THRESHOLD = 1000; // 并行阈值

    for (int n : { 100, 5000 }) { // 测试小数据和大数据
        int sum = 0;

// 核心: 只有 n >= THRESHOLD 时才并行
#pragma omp parallel for reduction(+ : sum) if (n >= THRESHOLD)
        for (int i = 0; i < n; i++) {
            sum += i;
        }

        // 验证执行模式
        bool is_parallel = (n >= THRESHOLD);
        std::cout << "  n=" << std::setw(5) << n << " → " << (is_parallel ? "并行" : "串行")
                  << "执行, sum=" << sum << "\n";
    }

    // ========================================================================
    // 场景 2: 运行时条件判断
    // ========================================================================
    std::cout << "\n【场景 2】运行时条件判断\n";
    std::cout << "原理: 根据运行时状态动态决定是否并行\n\n";

    for (bool enable_parallel : { false, true }) {
#pragma omp parallel if (enable_parallel)
        {
#pragma omp single
            {
                std::cout << "  enable_parallel=" << std::boolalpha << enable_parallel
                          << " → 线程数: " << omp_get_num_threads() << "\n";
            }
        }
    }

    // ========================================================================
    // 场景 3: 调试模式 - 快速禁用并行
    // ========================================================================
    std::cout << "\n【场景 3】调试模式开关\n";
    std::cout << "原理: 调试时禁用并行，便于定位问题\n\n";

    constexpr bool DEBUG_MODE = true; // 编译期常量，可优化

#pragma omp parallel if (!DEBUG_MODE)
    {
#pragma omp single
        {
            std::cout << "  DEBUG_MODE=" << DEBUG_MODE << " → 线程数: " << omp_get_num_threads()
                      << " (调试时强制串行)\n";
        }
    }

    // ========================================================================
    // 场景 4: 与其他子句组合使用
    // ========================================================================
    std::cout << "\n【场景 4】与其他子句组合\n";
    std::cout << "原理: if 可与 num_threads、reduction 等组合\n\n";

    int    data_size = 2000;
    double result    = 0.0;

// 组合: if + num_threads + reduction
#pragma omp parallel for if (data_size > 500) num_threads(2) reduction(+ : result)
    for (int i = 0; i < data_size; i++) {
        result += i * 0.001;
    }
    std::cout << "  if + num_threads + reduction: result=" << result << "\n";

    // ========================================================================
    // 场景 5: sections 指令中使用 if
    // ========================================================================
    std::cout << "\n【场景 5】sections 中使用 if\n";
    std::cout << "原理: 任务数少时避免并行开销\n\n";

    int task_count = 2;

#pragma omp parallel sections if (task_count >= 4)
    {
#pragma omp section
        { std::cout << "  Section A - 线程 " << omp_get_thread_num() << "\n"; }
#pragma omp section
        { std::cout << "  Section B - 线程 " << omp_get_thread_num() << "\n"; }
    }
    std::cout << "  task_count=" << task_count << " < 4 → 串行执行 sections\n";

    // ========================================================================
    // 场景 6: 嵌套并行中的 if
    // ========================================================================
    std::cout << "\n【场景 6】嵌套并行中的 if\n";
    std::cout << "原理: 控制内层是否并行，避免线程爆炸\n\n";

    omp_set_nested(1);
    int outer_size = 100, inner_size = 50;

#pragma omp parallel num_threads(2) if (outer_size > 50)
    {
        int tid = omp_get_thread_num();

// 内层根据 inner_size 决定是否并行
#pragma omp parallel num_threads(2) if (inner_size > 100)
        {
#pragma omp single
            {
                std::cout << "  外层线程 " << tid << " 的内层线程数: " << omp_get_num_threads()
                          << " (inner_size=" << inner_size << " ≤ 100 → 串行)\n";
            }
        }
    }
    omp_set_nested(0);

    // ========================================================================
    // 场景 7: 表达式作为条件
    // ========================================================================
    std::cout << "\n【场景 7】复杂表达式作为条件\n";
    std::cout << "原理: 支持任意返回 bool 的表达式\n\n";

    int array_size        = 1500;
    int available_threads = omp_get_max_threads();

// 复杂条件: 数据量足够大 且 有多个可用线程
#pragma omp parallel if (array_size > 1000 && available_threads > 1)
    {
#pragma omp single
        {
            std::cout << "  条件: size(" << array_size << ")>1000 && threads(" << available_threads
                      << ")>1 → 线程数: " << omp_get_num_threads() << "\n";
        }
    }

    // ========================================================================
    // 总结: if 子句最佳实践
    // ========================================================================
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "【if 子句最佳实践】\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "1. 设置合理阈值: 根据实测确定并行收益点\n";
    std::cout << "2. 考虑并行开销: 线程创建约 10-100 微秒\n";
    std::cout << "3. 调试时禁用: if(!DEBUG) 便于问题定位\n";
    std::cout << "4. 嵌套时谨慎: 内层 if 避免线程数爆炸\n";
    std::cout << "5. 运行时决策: 结合 omp_get_max_threads()\n";
}

// ============================================================================
// MAIN 函数
// ============================================================================
int main() {
    std::cout << "============================================\n";
    std::cout << "OpenMP 设置并行区域线程数的四种方式\n";
    std::cout << "============================================\n";

    // 1. 默认方式
    demo_default_threads();

    // 2. 库函数方式
    demo_library_function();

    // 3. 指令方式
    demo_directive_clause();

    // 4. 环境变量方式
    demo_environment_variable();

    // 优先级演示
    demo_priority();

    // 嵌套并行模式
    demo_nested_parallelism();

    // 动态调整模式
    demo_dynamic_adjustment();

    // 实用示例
    demo_practical_example();

    // 三种模式总结
    print_mode_summary();

    // if 子句详解
    demo_if_clause();

    std::cout << "\n============================================\n";
    std::cout << "提示：运行前设置环境变量测试第 4 种方式：\n";
    std::cout << "  export OMP_NUM_THREADS=6\n";
    std::cout << "  ./openmp_use_parallel\n";
    std::cout << "============================================\n";

    return EXIT_SUCCESS;
}
