/**
 * @file openmp_data_sharing_examples.cpp
 * @brief OpenMP 数据共享机制全面示例
 *
 * 本文件演示了 OpenMP 中重要的数据共享指令：
 * 1. threadprivate - 线程私有变量
 * 2. copyin - 复制主线程值到线程私有变量
 * 3. copyprivate - 在线程间广播私有变量
 * 4. reduction - 规约操作
 *
 * 包含了常见的错误示例和正确的使用方法
 */

#include <omp.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// ============================================================================
// 1. THREADPRIVATE 示例
// ============================================================================

/**
 * threadprivate 将全局变量或静态变量声明为线程私有
 * 每个线程都有该变量的独立副本，在并行区域之间保持值
 */

// 全局变量声明为 threadprivate
int         global_thread_id = -1;
#pragma omp threadprivate(global_thread_id)

// 静态变量声明为 threadprivate
static double thread_data[10] = { 0 };
#pragma omp   threadprivate(thread_data)

void demo_threadprivate_basic() {
    std::cout << "\n========================================\n";
    std::cout << "1. THREADPRIVATE 基础示例\n";
    std::cout << "========================================\n";

// 第一个并行区域：初始化 threadprivate 变量
#pragma omp parallel num_threads(4)
    {
        int tid          = omp_get_thread_num();
        global_thread_id = tid; // 每个线程设置自己的私有副本
        thread_data[0]   = tid * 10.0;
        thread_data[1]   = tid * 100.0;

#pragma omp critical
        {
            std::cout << "线程 " << tid << " 设置 global_thread_id = " << global_thread_id
                      << ", thread_data[0] = " << thread_data[0] << "\n";
        }
    }

    // 第二个并行区域：threadprivate 变量保持各自的值
    std::cout << "\n第二个并行区域（threadprivate 变量保持值）：\n";
#pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
#pragma omp critical
        {
            std::cout << "线程 " << tid << " 读取 global_thread_id = " << global_thread_id
                      << ", thread_data[0] = " << thread_data[0] << "\n";
        }
    }
}

// ============================================================================
// 2. COPYIN 示例
// ============================================================================

/**
 * copyin 子句将主线程的 threadprivate 变量值复制到所有线程
 * 只能用于 threadprivate 变量
 */

int         master_value = 999;
#pragma omp threadprivate(master_value)

void demo_copyin() {
    std::cout << "\n========================================\n";
    std::cout << "2. COPYIN 示例\n";
    std::cout << "========================================\n";

    // 主线程设置初始值
    master_value = 42;
    std::cout << "主线程设置 master_value = " << master_value << "\n\n";

    // 不使用 copyin：每个线程的值是未定义的
    std::cout << "不使用 copyin（错误示例）：\n";
#pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
#pragma omp critical
        { std::cout << "线程 " << tid << " 的 master_value = " << master_value << " (未定义)\n"; }
    }

    // 重新设置主线程值
    master_value = 100;
    std::cout << "\n主线程重新设置 master_value = " << master_value << "\n\n";

    // 使用 copyin：主线程的值被复制到所有线程
    std::cout << "使用 copyin（正确示例）：\n";
#pragma omp parallel num_threads(4) copyin(master_value)
    {
        int tid = omp_get_thread_num();
#pragma omp critical
        { std::cout << "线程 " << tid << " 的 master_value = " << master_value << "\n"; }

        // 每个线程可以修改自己的副本
        master_value += tid;

#pragma omp barrier
#pragma omp critical
        { std::cout << "线程 " << tid << " 修改后 master_value = " << master_value << "\n"; }
    }
}

// ============================================================================
// 3. COPYPRIVATE 示例
// ============================================================================

/**
 * copyprivate 用于在 single 指令后将私有变量的值广播到所有线程
 * 必须与 single 指令配合使用
 */

void demo_copyprivate() {
    std::cout << "\n========================================\n";
    std::cout << "3. COPYPRIVATE 示例\n";
    std::cout << "========================================\n";

    int shared_data = 0;

#pragma omp parallel num_threads(4)
    {
        int tid           = omp_get_thread_num();
        int private_value = -1; // 每个线程的私有变量

// single 指令：只有一个线程执行
// copyprivate：将执行线程的 private_value 广播到所有线程
#pragma omp single copyprivate(private_value)
        {
            int executing_tid = omp_get_thread_num();
            private_value     = 888; // 只有执行线程设置这个值
            std::cout << "线程 " << executing_tid
                      << " 执行 single 区域，设置 private_value = " << private_value << "\n\n";
        }

// 所有线程现在都有相同的 private_value
#pragma omp critical
        { std::cout << "线程 " << tid << " 接收到 private_value = " << private_value << "\n"; }
    }

    std::cout << "\n使用场景：从文件读取配置（只读一次，分发给所有线程）\n";
#pragma omp parallel num_threads(4)
    {
        int    tid             = omp_get_thread_num();
        int    config_value    = 0;
        double config_array[5] = { 0 };

// 只有一个线程执行"读取文件"操作
#pragma omp single copyprivate(config_value, config_array)
        {
            // 模拟读取配置文件
            config_value = 12345;
            for (int i = 0; i < 5; i++) {
                config_array[i] = i * 1.5;
            }
            std::cout << "\n线程 " << tid << " 读取配置文件\n";
        }

// 所有线程都可以使用配置
#pragma omp critical
        {
            std::cout << "线程 " << tid << " 使用配置: config_value = " << config_value
                      << ", config_array[2] = " << config_array[2] << "\n";
        }
    }
}

/**
 * 常见错误：在没有 single 的情况下使用 copyprivate
 */
void demo_copyprivate_error() {
    std::cout << "\n========================================\n";
    std::cout << "3.1 COPYPRIVATE 常见错误\n";
    std::cout << "========================================\n";

    std::cout << "错误：copyprivate 必须与 single 配合使用\n";
    std::cout << "// 以下代码会导致编译错误：\n";
    std::cout << "// #pragma omp parallel copyprivate(value)\n";
    std::cout << "// 正确的用法必须是：\n";
    std::cout << "// #pragma omp single copyprivate(value)\n";
}

// ============================================================================
// 4. REDUCTION 示例
// ============================================================================

/**
 * reduction 子句用于对变量进行规约操作
 * 支持的操作符：+, -, *, &, |, ^, &&, ||, max, min
 */

void demo_reduction_sum() {
    std::cout << "\n========================================\n";
    std::cout << "4. REDUCTION 基础示例 - 求和\n";
    std::cout << "========================================\n";

    const int        N = 1000;
    std::vector<int> data(N);

    // 初始化数据
    for (int i = 0; i < N; i++) {
        data[i] = i + 1; // 1, 2, 3, ..., 1000
    }

    // 串行版本（用于验证）
    int serial_sum = 0;
    for (int i = 0; i < N; i++) {
        serial_sum += data[i];
    }
    std::cout << "串行求和结果: " << serial_sum << "\n";

    // 错误示例：不使用 reduction（会产生竞争条件）
    int wrong_sum = 0;
#pragma omp parallel for num_threads(4)
    for (int i = 0; i < N; i++) {
        wrong_sum += data[i]; // 竞争条件！结果不正确
    }
    std::cout << "不使用 reduction（错误）: " << wrong_sum << " (结果不确定)\n";

    // 正确示例：使用 reduction
    int correct_sum = 0;
#pragma omp parallel for num_threads(4) reduction(+ : correct_sum)
    for (int i = 0; i < N; i++) {
        correct_sum += data[i];
    }
    std::cout << "使用 reduction（正确）: " << correct_sum << "\n";
}

void demo_reduction_operators() {
    std::cout << "\n========================================\n";
    std::cout << "4.1 REDUCTION 各种操作符\n";
    std::cout << "========================================\n";

    const int           N = 100;
    std::vector<double> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = i + 1.0;
    }

    // 1. 加法 reduction (+)
    double sum = 0.0;
#pragma omp parallel for reduction(+ : sum)
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    std::cout << "加法 reduction (+): sum = " << sum << "\n";

    // 2. 乘法 reduction (*)
    double product = 1.0;
#pragma omp parallel for reduction(* : product)
    for (int i = 0; i < 10; i++) { // 只用前10个，避免溢出
        product *= (i + 1);
    }
    std::cout << "乘法 reduction (*): product = " << product << "\n";

    // 3. 最大值 reduction (max)
    double max_val = data[0];
#pragma omp parallel for reduction(max : max_val)
    for (int i = 0; i < N; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    std::cout << "最大值 reduction (max): max_val = " << max_val << "\n";

    // 4. 最小值 reduction (min)
    double min_val = data[0];
#pragma omp parallel for reduction(min : min_val)
    for (int i = 0; i < N; i++) {
        if (data[i] < min_val) {
            min_val = data[i];
        }
    }
    std::cout << "最小值 reduction (min): min_val = " << min_val << "\n";

    // 5. 逻辑与 reduction (&&)
    std::vector<bool> flags(N, true);
    flags[50] = false; // 设置一个为 false

    bool all_true = true;
#pragma omp parallel for reduction(&& : all_true)
    for (int i = 0; i < N; i++) {
        all_true = all_true && flags[i];
    }
    std::cout << "逻辑与 reduction (&&): all_true = " << (all_true ? "true" : "false") << "\n";

    // 6. 逻辑或 reduction (||)
    bool any_false = false;
#pragma omp parallel for reduction(|| : any_false)
    for (int i = 0; i < N; i++) {
        any_false = any_false || !flags[i];
    }
    std::cout << "逻辑或 reduction (||): any_false = " << (any_false ? "true" : "false") << "\n";

    // 7. 位与 reduction (&)
    int bit_and = 0xFF; // 11111111
#pragma omp parallel for reduction(& : bit_and)
    for (int i = 0; i < 8; i++) {
        bit_and = bit_and & (0xFF - (1 << i));
    }
    std::cout << "位与 reduction (&): bit_and = " << bit_and << "\n";

    // 8. 位或 reduction (|)
    int bit_or = 0;
#pragma omp parallel for reduction(| : bit_or)
    for (int i = 0; i < 8; i++) {
        bit_or = bit_or | (1 << i);
    }
    std::cout << "位或 reduction (|): bit_or = 0x" << std::hex << bit_or << std::dec << "\n";
}

void demo_reduction_multiple_variables() {
    std::cout << "\n========================================\n";
    std::cout << "4.2 REDUCTION 多个变量同时规约\n";
    std::cout << "========================================\n";

    const int           N = 1000;
    std::vector<double> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = (i % 10) - 5.0; // -5 到 4 的循环
    }

    double sum            = 0.0;
    double sum_squares    = 0.0;
    int    count_positive = 0;
    int    count_negative = 0;

#pragma omp parallel for reduction(+ : sum, sum_squares, count_positive, count_negative)
    for (int i = 0; i < N; i++) {
        sum += data[i];
        sum_squares += data[i] * data[i];
        if (data[i] > 0) {
            count_positive++;
        }
        if (data[i] < 0) {
            count_negative++;
        }
    }

    double mean     = sum / N;
    double variance = (sum_squares / N) - (mean * mean);

    std::cout << "统计结果：\n";
    std::cout << "  和: " << sum << "\n";
    std::cout << "  平方和: " << sum_squares << "\n";
    std::cout << "  均值: " << mean << "\n";
    std::cout << "  方差: " << variance << "\n";
    std::cout << "  正数个数: " << count_positive << "\n";
    std::cout << "  负数个数: " << count_negative << "\n";
}

void demo_reduction_with_private() {
    std::cout << "\n========================================\n";
    std::cout << "4.3 REDUCTION 与 PRIVATE 结合使用\n";
    std::cout << "========================================\n";

    const int        N = 100;
    std::vector<int> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }

    int total = 0;

#pragma omp parallel for reduction(+ : total)
    for (int i = 0; i < N; i++) {
        // 中间计算变量自动是私有的
        int square = data[i] * data[i];
        int cube   = square * data[i];

        // 只有 total 参与 reduction
        total += cube;

        // 演示：每个线程看到的是自己的 total 副本
        if (i < 5) {
#pragma omp critical
            {
                std::cout << "线程 " << omp_get_thread_num() << " 处理 i=" << i
                          << ", 当前累加值: " << total << "\n";
            }
        }
    }

    std::cout << "最终 total = " << total << "\n";
}

// ============================================================================
// 5. 复杂场景示例
// ============================================================================

void demo_combined_usage() {
    std::cout << "\n========================================\n";
    std::cout << "5. 组合使用示例\n";
    std::cout << "========================================\n";

    // 场景：并行处理数据，使用 threadprivate 存储线程局部信息

    static int thread_processed_count = 0;
#pragma omp          threadprivate(thread_processed_count)

    const int           N = 1000;
    std::vector<double> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = i * 0.1;
    }

    double global_sum   = 0.0;
    int    global_count = 0;

    // 第一个并行区域：初始化 threadprivate
         #pragma omp parallel copyin(thread_processed_count)
    { thread_processed_count = 0; }

    // 第二个并行区域：处理数据
         #pragma omp parallel for reduction(+ : global_sum, global_count)
    for (int i = 0; i < N; i++) {
        if (data[i] > 50.0) {
            global_sum += data[i];
            global_count++;
            thread_processed_count++; // 每个线程统计自己处理的数量
        }
    }

    std::cout << "符合条件的元素总和: " << global_sum << "\n";
    std::cout << "符合条件的元素个数: " << global_count << "\n";

    // 第三个并行区域：查看各线程处理量
    std::cout << "\n各线程处理的数据量：\n";
         #pragma omp parallel
    {
        int tid = omp_get_thread_num();
#pragma omp critical
        { std::cout << "线程 " << tid << " 处理了 " << thread_processed_count << " 个元素\n"; }
    }
}

// ============================================================================
// 6. 常见错误总结
// ============================================================================

void demo_common_errors() {
    std::cout << "\n========================================\n";
    std::cout << "6. 常见错误总结\n";
    std::cout << "========================================\n";

    std::cout << "\n错误 1: 对非 threadprivate 变量使用 copyin\n";
    std::cout << "// int normal_var = 10;\n";
    std::cout << "// #pragma omp parallel copyin(normal_var)  // 编译错误！\n";
    std::cout << "// copyin 只能用于 threadprivate 变量\n";

    std::cout << "\n错误 2: 不使用 reduction 导致竞争条件\n";
    std::cout << "// int sum = 0;\n";
    std::cout << "// #pragma omp parallel for\n";
    std::cout << "// for (int i = 0; i < N; i++) {\n";
    std::cout << "//     sum += array[i];  // 竞争条件！\n";
    std::cout << "// }\n";

    std::cout << "\n错误 3: reduction 变量初始值不正确\n";
    std::cout << "// 对于加法，初始值应为 0\n";
    std::cout << "// 对于乘法，初始值应为 1\n";
    std::cout << "// 对于最大值，初始值应为最小可能值\n";
    std::cout << "// 对于最小值，初始值应为最大可能值\n";

    std::cout << "\n错误 4: threadprivate 变量在第一次并行区域前未初始化\n";
    std::cout << "// 如果不使用 copyin，每个线程的 threadprivate 变量\n";
    std::cout << "// 值是未定义的，必须显式初始化\n";

    std::cout << "\n错误 5: 混淆 private 和 threadprivate\n";
    std::cout << "// private: 在并行区域内私有，区域结束后值丢失\n";
    std::cout << "// threadprivate: 线程私有，跨并行区域保持值\n";

    std::cout << "\n错误 6: reduction 操作符与初始值不匹配\n";
    int              wrong_max = 0; // 错误：对于 max，应该用最小值初始化
    std::vector<int> test_data = { -10, -5, -2, -8 };
#pragma omp parallel for reduction(max : wrong_max)
    for (size_t i = 0; i < test_data.size(); i++) {
        if (test_data[i] > wrong_max) {
            wrong_max = test_data[i];
        }
    }
    std::cout << "\n错误的最大值查找（初始值为0）: " << wrong_max << " (应该是 -2，但得到 0)\n";

    int correct_max = test_data[0]; // 正确：用第一个元素或极小值初始化
#pragma omp parallel for reduction(max : correct_max)
    for (size_t i = 0; i < test_data.size(); i++) {
        if (test_data[i] > correct_max) {
            correct_max = test_data[i];
        }
    }
    std::cout << "正确的最大值查找（初始值为第一个元素）: " << correct_max << "\n";
}

// ============================================================================
// 7. 性能对比示例
// ============================================================================

void demo_performance_comparison() {
    std::cout << "\n========================================\n";
    std::cout << "7. 性能对比：reduction vs critical\n";
    std::cout << "========================================\n";

    const int           N = 10000000;
    std::vector<double> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = i * 0.1;
    }

    // 方法 1: 使用 reduction（推荐）
    double sum1   = 0.0;
    double start1 = omp_get_wtime();
#pragma omp parallel for reduction(+ : sum1)
    for (int i = 0; i < N; i++) {
        sum1 += data[i];
    }
    double end1 = omp_get_wtime();
    std::cout << "使用 reduction: sum = " << sum1 << ", 耗时: " << (end1 - start1) * 1000
              << " ms\n";

    // 方法 2: 使用 critical（不推荐，性能差）
    double sum2   = 0.0;
    double start2 = omp_get_wtime();
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
#pragma omp critical
        { sum2 += data[i]; }
    }
    double end2 = omp_get_wtime();
    std::cout << "使用 critical: sum = " << sum2 << ", 耗时: " << (end2 - start2) * 1000 << " ms\n";

    std::cout << "\n性能提升: " << (end2 - start2) / (end1 - start1) << " 倍\n";
    std::cout << "结论: reduction 比 critical 快得多，应优先使用 reduction\n";
}

// ============================================================================
// MAIN 函数
// ============================================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "OpenMP 数据共享机制全面示例\n";
    std::cout << "============================================\n";

    // 设置输出精度
    std::cout << std::fixed << std::setprecision(2);

    // 显示 OpenMP 版本和线程数
#ifdef _OPENMP
    std::cout << "OpenMP 版本: " << _OPENMP << "\n";
#else
    std::cout << "OpenMP 版本: Disabled\n";
#endif
    std::cout << "可用线程数: " << omp_get_max_threads() << "\n";

    // 1. threadprivate 示例
    demo_threadprivate_basic();

    // 2. copyin 示例
    demo_copyin();

    // 3. copyprivate 示例
    demo_copyprivate();
    demo_copyprivate_error();

    // 4. reduction 示例
    demo_reduction_sum();
    demo_reduction_operators();
    demo_reduction_multiple_variables();
    demo_reduction_with_private();

    // 5. 组合使用示例
    demo_combined_usage();

    // 6. 常见错误
    demo_common_errors();

    // 7. 性能对比
    demo_performance_comparison();

    std::cout << "\n============================================\n";
    std::cout << "示例执行完成！\n";
    std::cout << "============================================\n";

    return 0;
}
