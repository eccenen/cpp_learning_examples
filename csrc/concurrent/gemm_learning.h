#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace concurrent {

/**
 * @brief 矩阵类 - 使用行主序存储
 *
 * 行主序(Row-major): 内存中按行连续存储
 * 对于 M[i][j]，其在一维数组中的位置为: i * cols + j
 */
class Matrix {
  public:
    Matrix(size_t rows, size_t cols, double init_val = 0.0);

    // 访问元素 - 提供边界检查
    double &       operator()(size_t i, size_t j);
    const double & operator()(size_t i, size_t j) const;

    // 获取矩阵维度
    size_t rows() const { return rows_; }

    size_t cols() const { return cols_; }

    // 获取原始数据指针（用于性能优化）
    double * data() { return data_.data(); }

    const double * data() const { return data_.data(); }

    // 随机初始化矩阵
    void randomize(double min = 0.0, double max = 1.0);

    // 验证两个矩阵是否相等（用于正确性检查）
    bool equals(const Matrix & other, double epsilon = 1e-6) const;

  private:
    size_t              rows_;
    size_t              cols_;
    std::vector<double> data_;
};

/**
 * ============================================================================
 * 串行版本 GEMM (General Matrix Multiply)
 * ============================================================================
 */

/**
 * @brief 基础三重循环实现 - 最直观的实现方式
 *
 * 算法: C = A * B
 * 时间复杂度: O(M*N*K)
 *
 * 教学要点:
 * - 基础实现，循环顺序为 i-j-k
 * - 内存访问模式不友好，缓存命中率低
 * - B矩阵列访问导致cache miss
 */
void gemm_serial_naive(const Matrix & A, const Matrix & B, Matrix & C);

/**
 * @brief 缓存优化版本 - 使用分块(Tiling/Blocking)技术
 *
 * 教学要点:
 * - 分块提高缓存局部性
 * - 减少cache miss，提升性能
 * - 循环顺序优化: i-k-j 或 分块后的优化顺序
 *
 * @param block_size 分块大小，通常选择32/64/128等2的幂次
 */
void gemm_serial_blocked(const Matrix & A, const Matrix & B, Matrix & C, size_t block_size = 64);

/**
 * ============================================================================
 * 并行版本 GEMM - 使用 std::thread
 * ============================================================================
 */

/**
 * @brief 使用std::thread的基础并行实现 - 按行分块
 *
 * 教学要点:
 * - 数据并行: 将C矩阵的行分配给不同线程
 * - 无数据竞争: 每个线程写入独立的C矩阵行
 * - 负载均衡: 简单的行分配可能不均衡
 * - 线程创建/销毁开销
 *
 * @param num_threads 线程数量
 */
void gemm_thread_parallel(const Matrix & A, const Matrix & B, Matrix & C, size_t num_threads = 4);

/**
 * @brief 使用std::thread的改进版本 - 分块+并行
 *
 * 教学要点:
 * - 结合缓存优化和并行
 * - 更好的负载均衡: 按块分配而非按行
 * - 同步开销: 线程创建和join的成本
 *
 * @param num_threads 线程数量
 * @param block_size 缓存分块大小
 */
void gemm_thread_blocked(const Matrix & A, const Matrix & B, Matrix & C, size_t num_threads = 4,
                         size_t block_size = 64);

/**
 * @brief 错误示范 - 展示数据竞争问题
 *
 * 教学要点:
 * - 故意制造数据竞争
 * - 展示无保护的共享变量访问
 * - 结果不确定性
 *
 * 注意: 此函数仅用于教学展示，不应在生产代码中使用！
 */
void gemm_thread_race_condition_demo(const Matrix & A, const Matrix & B, Matrix & C,
                                     size_t num_threads = 4);

/**
 * ============================================================================
 * 并行版本 GEMM - 使用 OpenMP
 * ============================================================================
 */

/**
 * @brief OpenMP并行循环版本 - 最简单的并行化方式
 *
 * 教学要点:
 * - #pragma omp parallel for 自动并行化
 * - 编译器处理线程管理和负载均衡
 * - 同步开销: parallel region的创建成本
 * - schedule策略: static vs dynamic vs guided
 *
 * @param schedule_type "static", "dynamic", "guided"
 */
void gemm_openmp_simple(const Matrix & A, const Matrix & B, Matrix & C,
                        const std::string & schedule_type = "static");

/**
 * @brief OpenMP + 分块优化
 *
 * 教学要点:
 * - 结合缓存优化和OpenMP并行
 * - collapse子句合并循环
 * - 线程局部性优化
 */
void gemm_openmp_blocked(const Matrix & A, const Matrix & B, Matrix & C, size_t block_size = 64);

/**
 * ============================================================================
 * 线程池实现
 * ============================================================================
 */

/**
 * @brief 简单的线程池实现
 *
 * 教学要点:
 * - 避免重复创建/销毁线程
 * - 任务队列 + 工作线程
 * - 条件变量同步
 * - 线程生命周期管理
 */
class ThreadPool {
  public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    // 禁止拷贝和移动
    ThreadPool(const ThreadPool &)             = delete;
    ThreadPool & operator=(const ThreadPool &) = delete;

    // 提交任务到线程池
    template <typename Func> void enqueue(Func && task);

    // 等待所有任务完成
    void wait();

    size_t size() const { return threads_.size(); }

  private:
    void worker_thread();

    std::vector<std::thread>          threads_;
    std::queue<std::function<void()>> tasks_;

    std::mutex              queue_mutex_;
    std::condition_variable condition_;
    std::atomic<size_t>     active_tasks_{ 0 };
    std::atomic<bool>       stop_{ false };
};

/**
 * @brief 使用线程池的GEMM实现
 *
 * 教学要点:
 * - 线程复用，避免创建开销
 * - 任务粒度控制
 * - 与直接使用thread的性能对比
 *
 * @param pool 线程池引用
 * @param task_granularity 每个任务处理的行数
 */
void gemm_threadpool(ThreadPool & pool, const Matrix & A, const Matrix & B, Matrix & C,
                     size_t task_granularity = 16);

/**
 * ============================================================================
 * 性能测试和验证工具
 * ============================================================================
 */

/**
 * @brief 性能计时器
 */
class Timer {
  public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start_).count();
    }

    void reset() { start_ = std::chrono::high_resolution_clock::now(); }

  private:
    std::chrono::high_resolution_clock::time_point start_;
};

/**
 * @brief 性能统计结构
 */
struct PerformanceResult {
    std::string method_name;
    double      time_seconds;
    double      gflops; // 十亿次浮点运算/秒
    bool        is_correct;

    void print() const;
};

/**
 * @brief 运行单个GEMM测试并返回性能结果
 */
template <typename GemmFunc, typename... Args>
PerformanceResult benchmark_gemm(const std::string & name, GemmFunc && func, const Matrix & A,
                                 const Matrix & B, const Matrix & reference, Args &&... args);

// Template implementation must be visible to users of the header (define here)
template <typename GemmFunc, typename... Args>
PerformanceResult benchmark_gemm(const std::string & name, GemmFunc && func, const Matrix & A,
                                 const Matrix & B, const Matrix & reference, Args &&... args) {
    Timer  t;
    Matrix C(A.rows(), B.cols(), 0.0);
    // Call the provided GEMM implementation (may accept extra args)
    func(A, B, C, std::forward<Args>(args)...);
    double elapsed = t.elapsed();
    double ops     = 2.0 * static_cast<double>(A.rows()) * static_cast<double>(A.cols()) *
                 static_cast<double>(B.cols());
    double gflops = (ops / 1e9) / elapsed;
    bool   ok     = C.equals(reference);
    return PerformanceResult{ name, elapsed, gflops, ok };
}

} // namespace concurrent
