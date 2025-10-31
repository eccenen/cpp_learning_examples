#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

#include "gemm_learning.h"

#ifdef _OPENMP
#    include <omp.h>
#endif

namespace concurrent {

// ==================== Matrix 类实现 ====================

Matrix::Matrix(size_t rows, size_t cols, double init_val) :
    rows_(rows),
    cols_(cols),
    data_(rows * cols, init_val) {}

double & Matrix::operator()(size_t i, size_t j) {
    return data_[i * cols_ + j];
}

const double & Matrix::operator()(size_t i, size_t j) const {
    return data_[i * cols_ + j];
}

void Matrix::randomize(double min, double max) {
    std::mt19937_64                        rng(12345);
    std::uniform_real_distribution<double> dist(min, max);
    for (auto & v : data_) {
        v = dist(rng);
    }
}

bool Matrix::equals(const Matrix & other, double epsilon) const {
    if (rows_ != other.rows() || cols_ != other.cols()) {
        return false;
    }
    for (size_t i = 0; i < data_.size(); ++i) {
        if (std::fabs(data_[i] - other.data()[i]) > epsilon) {
            return false;
        }
    }
    return true;
}

// ==================== 串行版本：基础实现 ====================
// 教学要点：最直接的三层循环，性能基准
// 时间复杂度：O(M*N*K)，无优化
void gemm_serial_naive(const Matrix & A, const Matrix & B, Matrix & C) {
    size_t M = A.rows(), K = A.cols(), N = B.cols();

    // 标准的矩阵乘法：C[i][j] = sum(A[i][k] * B[k][j])
    for (size_t i = 0; i < M; ++i) { // 遍历A的行
        for (size_t j = 0; j < N; ++j) { // 遍历B的列
            double sum = 0.0;
            for (size_t k = 0; k < K; ++k) { // 内积计算
                sum += A(i, k) * B(k, j);
            }
            C(i, j) = sum;
        }
    }
    // 问题：内层循环访问B是列优先，导致大量cache miss
}

// ==================== 串行版本：分块优化（缓存友好）====================
// 教学要点：提高数据局部性，减少cache miss
// 优化原理：将大矩阵分成小块，每块能装入CPU缓存
void gemm_serial_blocked(const Matrix & A, const Matrix & B, Matrix & C, size_t block_size) {
    size_t M = A.rows(), K = A.cols(), N = B.cols();

    // 外层三重循环：按块遍历
    for (size_t ii = 0; ii < M; ii += block_size) { // A的行块
        for (size_t kk = 0; kk < K; kk += block_size) { // A的列块/B的行块
            for (size_t jj = 0; jj < N; jj += block_size) { // B的列块

                // 计算当前块的实际大小（处理边界）
                size_t i_max = std::min(ii + block_size, M);
                size_t k_max = std::min(kk + block_size, K);
                size_t j_max = std::min(jj + block_size, N);

                // 内层三重循环：在块内计算
                for (size_t i = ii; i < i_max; ++i) {
                    for (size_t k = kk; k < k_max; ++k) {
                        double a_ik = A(i, k); // 复用A元素
                        for (size_t j = jj; j < j_max; ++j) {
                            C(i, j) += a_ik * B(k, j); // 累加到C
                        }
                    }
                }
            }
        }
    }
    // 优化效果：减少内存访问延迟，典型提速2-5倍
}

// ==================== 并行版本1：std::thread 行分块 ====================
// 教学要点：手动创建线程、负载均衡、线程同步开销

// 工作线程函数：计算C矩阵的[row_begin, row_end)行
static void gemm_worker_rows(const Matrix & A, const Matrix & B, Matrix & C, size_t row_begin,
                             size_t row_end) {
    size_t K = A.cols(), N = B.cols();

    // 每个线程独立计算分配给它的行，无数据竞争
    for (size_t i = row_begin; i < row_end; ++i) {
        for (size_t j = 0; j < N; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < K; ++k) {
                sum += A(i, k) * B(k, j);
            }
            C(i, j) = sum;
        }
    }
}

void gemm_thread_parallel(const Matrix & A, const Matrix & B, Matrix & C, size_t num_threads) {
    size_t M    = A.rows();
    num_threads = std::max<size_t>(1, std::min(num_threads, M));

    std::vector<std::thread> threads;
    size_t                   base   = M / num_threads; // 每线程基础行数
    size_t                   rem    = M % num_threads; // 余数行
    size_t                   offset = 0;

    // 负载均衡：前rem个线程多分配1行
    for (size_t t = 0; t < num_threads; ++t) {
        size_t rows  = base + (t < rem ? 1 : 0);
        size_t begin = offset;
        size_t end   = begin + rows;

        // 创建线程，传递只读的A、B和可写的C
        threads.emplace_back(gemm_worker_rows, std::cref(A), std::cref(B), std::ref(C), begin, end);
        offset = end;
    }

    // 等待所有线程完成（同步点）
    for (auto & th : threads) {
        th.join(); // 阻塞直到线程结束
    }
    // 注意：创建/销毁线程有开销，小矩阵可能不如串行快
}

// ==================== 并行版本2：std::thread + 分块优化 ====================
// 教学要点：结合缓存优化和并行化

static void gemm_worker_block(const Matrix & A, const Matrix & B, Matrix & C, size_t ii,
                              size_t i_max, size_t block_size) {
    size_t K = A.cols(), N = B.cols();

    // 每个线程在自己的行范围内做分块计算
    for (size_t kk = 0; kk < K; kk += block_size) {
        for (size_t jj = 0; jj < N; jj += block_size) {
            size_t k_max = std::min(kk + block_size, K);
            size_t j_max = std::min(jj + block_size, N);

            for (size_t i = ii; i < i_max; ++i) {
                for (size_t k = kk; k < k_max; ++k) {
                    double a_ik = A(i, k);
                    for (size_t j = jj; j < j_max; ++j) {
                        C(i, j) += a_ik * B(k, j);
                    }
                }
            }
        }
    }
}

void gemm_thread_blocked(const Matrix & A, const Matrix & B, Matrix & C, size_t num_threads,
                         size_t block_size) {
    size_t M    = A.rows();
    num_threads = std::max<size_t>(1, std::min(num_threads, M));

    std::vector<std::thread> threads;
    size_t                   base   = M / num_threads;
    size_t                   rem    = M % num_threads;
    size_t                   offset = 0;

    for (size_t t = 0; t < num_threads; ++t) {
        size_t rows  = base + (t < rem ? 1 : 0);
        size_t begin = offset;
        size_t end   = begin + rows;

        threads.emplace_back(gemm_worker_block, std::cref(A), std::cref(B), std::ref(C), begin, end,
                             block_size);
        offset = end;
    }

    for (auto & th : threads) {
        th.join();
    }
}

// ==================== 数据竞争演示（错误示范）====================
// 教学要点：展示不加同步保护时的并发错误
void gemm_thread_race_condition_demo(const Matrix & A, const Matrix & B, Matrix & C,
                                     size_t num_threads) {
    size_t M = A.rows(), N = B.cols();
    num_threads = std::max<size_t>(1, std::min(num_threads, M * N));

    std::vector<std::thread> threads;

    // 危险操作：多个线程同时修改同一个C(i,j)
    auto race_task = [&](size_t tid) {
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                for (size_t k = 0; k < A.cols(); ++k) {
                    // 数据竞争：读-改-写操作不是原子的
                    C(i, j) += A(i, k) * B(k, j);
                    // 可能导致：部分更新丢失，结果错误
                }
            }
        }
    };

    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back(race_task, t);
    }

    for (auto & th : threads) {
        th.join();
    }
    // 预期结果：C的值不正确，验证会失败
}

// ==================== 并行版本3：OpenMP简单实现 ====================
// 教学要点：编译器自动并行化，更简洁的代码
void gemm_openmp_simple(const Matrix & A, const Matrix & B, Matrix & C,
                        const std::string & schedule_type) {
#ifdef _OPENMP
    size_t M = A.rows(), N = B.cols(), K = A.cols();

// OpenMP并行for：自动分配迭代到线程
// schedule(static): 编译时静态分配，适合负载均匀的循环
#    pragma omp parallel for schedule(static)
    for (int i = 0; i < (int) M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < K; ++k) {
                sum += A(i, k) * B(k, j);
            }
            C(i, j) = sum;
        }
    }
    // 优点：代码简洁，编译器优化，开销小
    // 注意：需要编译时加 -fopenmp 标志
#else
    // 未启用OpenMP时回退到串行版本
    gemm_serial_naive(A, B, C);
#endif
}

// ==================== 并行版本4：OpenMP + 分块 ====================
void gemm_openmp_blocked(const Matrix & A, const Matrix & B, Matrix & C, size_t block_size) {
#ifdef _OPENMP
    size_t M = A.rows(), K = A.cols(), N = B.cols();

// 并行化外层块循环
#    pragma omp parallel for schedule(static)
    for (int ii = 0; ii < (int) M; ii += (int) block_size) {
        for (size_t kk = 0; kk < K; kk += block_size) {
            for (size_t jj = 0; jj < N; jj += block_size) {
                size_t i_max = std::min<size_t>(ii + block_size, M);
                size_t k_max = std::min(kk + block_size, K);
                size_t j_max = std::min(jj + block_size, N);

                for (size_t i = ii; i < i_max; ++i) {
                    for (size_t k = kk; k < k_max; ++k) {
                        double a_ik = A(i, k);
                        for (size_t j = jj; j < j_max; ++j) {
                            C(i, j) += a_ik * B(k, j);
                        }
                    }
                }
            }
        }
    }
#else
    gemm_serial_blocked(A, B, C, block_size);
#endif
}

// ==================== 简单线程池实现 ====================
// 教学要点：任务队列、工作线程、避免重复创建线程开销

ThreadPool::ThreadPool(size_t num_threads) {
    size_t n = std::max<size_t>(1, num_threads);

    // 创建工作线程，每个线程运行worker_thread函数
    for (size_t i = 0; i < n; ++i) {
        threads_.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool() {
    // 通知所有线程停止
    stop_.store(true);
    condition_.notify_all();

    // 等待所有线程退出
    for (auto & t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

// 工作线程主循环
void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // 等待条件：有任务或收到停止信号
            condition_.wait(lock, [this] { return stop_.load() || !tasks_.empty(); });

            // 退出条件：停止且队列为空
            if (stop_.load() && tasks_.empty()) {
                return;
            }

            // 取出任务
            task = std::move(tasks_.front());
            tasks_.pop();
            active_tasks_.fetch_add(1);
        }

        // 在锁外执行任务（避免阻塞其他线程）
        task();

        // 任务完成，计数减1
        active_tasks_.fetch_sub(1);
        condition_.notify_all();
    }
}

// 添加任务到队列
template <typename Func> void ThreadPool::enqueue(Func && task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        tasks_.emplace(std::forward<Func>(task));
    }
    condition_.notify_one(); // 唤醒一个等待的线程
}

// 等待所有任务完成
void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    condition_.wait(lock, [this] { return tasks_.empty() && active_tasks_.load() == 0; });
}

// 显式实例化
template void ThreadPool::enqueue<std::function<void()>>(std::function<void()> && task);

// ==================== 使用线程池的GEMM ====================
// 教学要点：任务粒度控制、避免线程创建开销
void gemm_threadpool(ThreadPool & pool, const Matrix & A, const Matrix & B, Matrix & C,
                     size_t task_granularity) {
    size_t M    = A.rows();
    size_t base = std::max<size_t>(1, task_granularity);

    // 将矩阵C按行分成多个任务
    for (size_t i = 0; i < M; i += base) {
        size_t i_end = std::min(i + base, M);

        // 提交任务到线程池（lambda捕获）
        pool.enqueue([=, &A, &B, &C]() { gemm_worker_block(A, B, C, i, i_end, 64); });
    }

    // 等待所有任务完成
    pool.wait();
    // 优势：线程复用，减少创建/销毁开销
}

// ==================== 性能结果打印 ====================
void PerformanceResult::print() const {
    std::cout << method_name << ": time=" << time_seconds << "s, " << "GFLOPS=" << gflops << ", "
              << "correct=" << (is_correct ? "yes" : "NO") << std::endl;
}

} // namespace concurrent
