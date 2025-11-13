#include <iomanip>
#include <iostream>
#include <vector>

#include "gemm_learning.h"

using namespace concurrent;

int main() {
    std::cout << "========== 并发编程学习：矩阵乘法(GEMM)示例 ==========\n\n";

    // 测试不同规模的矩阵
    std::vector<size_t> sizes = { 128, 256, 512 };

    for (auto M : sizes) {
        std::cout << "\n测试矩阵规模: " << M << "x" << M << "\n";
        std::cout << std::string(60, '-') << "\n";

        // 初始化矩阵
        Matrix A(M, M), B(M, M), C_ref(M, M);
        A.randomize(-1.0, 1.0);
        B.randomize(-1.0, 1.0);

        // 1. 串行基准版本（用于正确性验证）
        std::cout << "1. 串行基准版本（naive）...\n";
        // 先计算参考结果（一次），用于后续正确性验证
        gemm_serial_naive(A, B, C_ref);
        // 基准测试：传入函数指针（匹配签名），以及 A,B,reference
        auto result_serial = benchmark_gemm("Serial Naive", gemm_serial_naive, A, B, C_ref);
        result_serial.print();

        // 2. 串行分块优化
        std::cout << "\n2. 串行分块优化（cache-friendly）...\n";
        Matrix C_blocked(M, M);
        auto   result_blocked =
            benchmark_gemm("Serial Blocked", gemm_serial_blocked, A, B, C_blocked, (size_t) 64);
        result_blocked.print();
        std::cout << "   加速比: " << std::fixed << std::setprecision(2)
                  << result_serial.time_seconds / result_blocked.time_seconds << "x\n";

        // 3. std::thread并行
        std::cout << "\n3. std::thread 并行（4线程）...\n";
        Matrix C_thread(M, M);
        auto   result_thread =
            benchmark_gemm("Thread Parallel", gemm_thread_parallel, A, B, C_thread, (size_t) 4);
        result_thread.print();
        std::cout << "   加速比: " << result_serial.time_seconds / result_thread.time_seconds
                  << "x\n";

        // 4. std::thread + 分块
        std::cout << "\n4. std::thread + 分块优化（4线程）...\n";
        Matrix C_thread_blocked(M, M);
        auto   result_tb = benchmark_gemm("Thread Blocked", gemm_thread_blocked, A, B,
                                          C_thread_blocked, (size_t) 4, (size_t) 64);
        result_tb.print();
        std::cout << "   加速比: " << result_serial.time_seconds / result_tb.time_seconds << "x\n";

        // 5. OpenMP简单版本
        std::cout << "\n5. OpenMP 并行...\n";
        Matrix C_omp(M, M);
        auto   result_omp =
            benchmark_gemm("OpenMP Simple", gemm_openmp_simple, A, B, C_omp, std::string("static"));
        result_omp.print();
        std::cout << "   加速比: " << result_serial.time_seconds / result_omp.time_seconds << "x\n";

        // 6. 线程池版本
        std::cout << "\n6. 线程池实现（4线程）...\n";
        ThreadPool pool(4);
        Matrix     C_pool(M, M);
        // 线程池的函数签名与benchmark需要的签名不同（需要先传入 ThreadPool&），
        // 因此传入一个签名匹配的 lambda：(const Matrix&, const Matrix&, Matrix&, size_t)
        auto       pool_lambda = [&](const Matrix & a, const Matrix & b, Matrix & c, size_t gran) {
            gemm_threadpool(pool, a, b, c, gran);
        };
        auto result_pool =
            benchmark_gemm("ThreadPool", pool_lambda, A, B, C_pool, (size_t) (M / 8));
        result_pool.print();
        std::cout << "   加速比: " << result_serial.time_seconds / result_pool.time_seconds
                  << "x\n";

        // 7. 数据竞争演示（仅小矩阵）
        if (M <= 128) {
            std::cout << "\n7. 数据竞争演示（错误示范）...\n";
            Matrix C_race(M, M);
            auto   result_race =
                benchmark_gemm("Race Condition (BUGGY)", gemm_thread_race_condition_demo, A, B,
                               C_race, (size_t) 4);
            result_race.print();
            std::cout << "   ⚠️  注意：此版本存在数据竞争，结果不正确！\n";
        }
    }

    std::cout << "\n========== 测试完成 ==========\n";
    std::cout << "\n关键学习点总结：\n";
    std::cout << "1. 缓存优化：分块可显著提升性能（减少cache miss）\n";
    std::cout << "2. 并行加速：多核利用可提升2-4倍性能\n";
    std::cout << "3. 线程开销：小规模任务可能因开销反而变慢\n";
    std::cout << "4. 数据竞争：无同步的共享写入会导致错误结果\n";
    std::cout << "5. OpenMP：更简洁，编译器优化好\n";
    std::cout << "6. 线程池：避免重复创建线程，适合多任务场景\n\n";

    return 0;
}
