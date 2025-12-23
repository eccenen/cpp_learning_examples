#include <omp.h>
#include <stdio.h>

namespace {

void example_num_threads() {
    int thread_id, num_threads;
    thread_id   = omp_get_thread_num();
    num_threads = omp_get_num_threads();
    printf("Before hello from thread %d out of %d threads\n", thread_id, num_threads);

#pragma omp parallel num_threads(4) private(thread_id, num_threads)
    {
        thread_id   = omp_get_thread_num();
        num_threads = omp_get_num_threads();
        printf("Hello from thread %d out of %d threads\n", thread_id, num_threads);
    }

    printf("After hello from thread %d out of %d threads\n", thread_id, num_threads);
}

void example_set_num_threads() {
    int thread_id, num_threads;
    thread_id   = omp_get_thread_num();
    num_threads = omp_get_num_threads();
    printf("Before hello from thread %d out of %d threads\n", thread_id, num_threads);

    omp_set_num_threads(8);
#pragma omp parallel private(thread_id, num_threads)
    {
        thread_id   = omp_get_thread_num();
        num_threads = omp_get_num_threads();
        printf("Hello from thread %d out of %d threads\n", thread_id, num_threads);
    }

    printf("After hello from thread %d out of %d threads\n", thread_id, num_threads);
}

// ==================== 避免数据竞争的方法示例 ====================

// 方法1: 使用 critical 指令
// critical 创建一个临界区，同一时间只允许一个线程执行该区域的代码
// 优点：简单易用，适用于任何代码块
// 缺点：性能开销较大，所有线程都需要排队等待
void example_critical() {
    int shared_sum = 0;
    int n          = 100;

    printf("\n=== Method 1: Using critical ===\n");
    printf("Calculating sum with critical section...\n");

#pragma omp parallel for shared(shared_sum, n)
    for (int i = 1; i <= n; i++) {
#pragma omp critical
        {
            // 临界区：同一时间只有一个线程可以执行这段代码
            shared_sum += i;
        }
    }

    printf("Final sum: %d (Expected: %d)\n", shared_sum, n * (n + 1) / 2);
}

// 方法2: 使用 atomic 指令
// atomic 用于保护单个内存操作，比 critical 更轻量级
// 优点：性能优于 critical，开销小
// 缺点：只能用于简单的数学运算（+, -, *, /, &, |, ^, <<, >>）
void example_atomic() {
    int shared_sum = 0;
    int n          = 100;

    printf("\n=== Method 2: Using atomic ===\n");
    printf("Calculating sum with atomic operation...\n");

#pragma omp parallel for shared(shared_sum, n)
    for (int i = 1; i <= n; i++) {
// atomic 指令确保下面的操作是原子性的
#pragma omp atomic
        shared_sum += i;
    }

    printf("Final sum: %d (Expected: %d)\n", shared_sum, n * (n + 1) / 2);
}

// 方法3: 使用 reduction 子句
// reduction 是最推荐的方法，让每个线程维护私有副本，最后自动合并
// 优点：性能最好，自动处理规约操作，代码简洁
// 缺点：只适用于特定的规约操作（+, -, *, &, |, ^, &&, ||, max, min）
void example_reduction() {
    int shared_sum = 0;
    int n          = 100;

    printf("\n=== Method 3: Using reduction (Recommended) ===\n");
    printf("Calculating sum with reduction clause...\n");

    // reduction(+:shared_sum) 表示：
    // 1. 每个线程创建 shared_sum 的私有副本，初始化为 0
    // 2. 每个线程独立计算自己的部分
    // 3. 并行区域结束时，自动将所有副本相加到 shared_sum
#pragma omp parallel for reduction(+ : shared_sum)
    for (int i = 1; i <= n; i++) {
        shared_sum += i; // 每个线程操作自己的私有副本，无需同步
    }

    printf("Final sum: %d (Expected: %d)\n", shared_sum, n * (n + 1) / 2);
}

// 方法4: 使用 OpenMP 锁（Lock）
// 锁提供了更细粒度的控制，可以手动控制锁的获取和释放
// 优点：灵活性高，可以跨多个代码块使用
// 缺点：需要手动管理，容易出错（忘记释放锁会导致死锁）
void example_lock() {
    int        shared_sum = 0;
    int        n          = 100;
    omp_lock_t lock;

    printf("\n=== Method 4: Using OpenMP Lock ===\n");
    printf("Calculating sum with explicit lock...\n");

    // 初始化锁
    omp_init_lock(&lock);

#pragma omp parallel for shared(shared_sum, n, lock)
    for (int i = 1; i <= n; i++) {
        // 获取锁（如果锁已被其他线程持有，当前线程会等待）
        omp_set_lock(&lock);

        // 受保护的代码段
        shared_sum += i;

        // 释放锁，允许其他线程获取
        omp_unset_lock(&lock);
    }

    // 销毁锁，释放资源
    omp_destroy_lock(&lock);

    printf("Final sum: %d (Expected: %d)\n", shared_sum, n * (n + 1) / 2);
}

// 方法5: 使用 ordered 指令
// ordered 确保代码块按照串行顺序执行（按迭代顺序）
// 优点：保证执行顺序，适用于需要按序输出或处理的场景
// 缺点：严重限制并行性，性能较差
void example_ordered() {
    int shared_sum = 0;
    int n          = 20; // 使用较小的 n 以便观察顺序

    printf("\n=== Method 5: Using ordered ===\n");
    printf("Calculating sum with ordered directive...\n");

// ordered 子句表示循环中包含 ordered 指令
#pragma omp parallel for ordered shared(shared_sum, n)
    for (int i = 1; i <= n; i++) {
// ordered 块确保按迭代顺序执行（即使线程并行执行循环）
#pragma omp ordered
        {
            shared_sum += i;
            printf("Thread %d processes i=%d, sum=%d\n", omp_get_thread_num(), i, shared_sum);
        }
    }

    printf("Final sum: %d (Expected: %d)\n", shared_sum, n * (n + 1) / 2);
}

// 方法6: 使用命名的 critical 区域
// 可以为不同的 critical 区域命名，不同名称的临界区可以并发执行
// 优点：允许多个互不干扰的临界区并发执行
// 缺点：需要正确规划临界区的命名
void example_named_critical() {
    int sum_even = 0;
    int sum_odd  = 0;
    int n        = 100;

    printf("\n=== Method 6: Using named critical sections ===\n");
    printf("Calculating separate sums for even and odd numbers...\n");

#pragma omp parallel for shared(sum_even, sum_odd, n)
    for (int i = 1; i <= n; i++) {
        if (i % 2 == 0) {
// 命名为 "even" 的临界区
#pragma omp critical(even)
            sum_even += i;
        } else {
// 命名为 "odd" 的临界区
// 这两个临界区可以并发执行，因为它们保护不同的数据
#pragma omp critical(odd)
            sum_odd += i;
        }
    }

    printf("Sum of even numbers: %d\n", sum_even);
    printf("Sum of odd numbers: %d\n", sum_odd);
    printf("Total sum: %d (Expected: %d)\n", sum_even + sum_odd, n * (n + 1) / 2);
}

// 方法7: 使用 barrier 同步
// barrier 让所有线程在某个点同步，等待所有线程都到达该点后再继续
// 优点：可以分阶段处理数据，确保阶段间的数据一致性
// 缺点：不直接防止竞争，需要配合其他方法使用
void example_barrier() {
    const int n = 10;
    int       data[n];
    int       result[n];

    printf("\n=== Method 7: Using barrier for synchronization ===\n");
    printf("Processing data in two stages with barrier...\n");

    // 初始化数据
    for (int i = 0; i < n; i++) {
        data[i] = i + 1;
    }

#pragma omp parallel shared(data, result, n)
    {
        // 第一阶段：每个线程处理部分数据
#pragma omp for
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2;
            printf("Thread %d: Phase 1, data[%d] = %d\n", omp_get_thread_num(), i, data[i]);
        }

// barrier 确保所有线程完成第一阶段后才进入第二阶段
#pragma omp barrier

        // 第二阶段：使用第一阶段的结果
#pragma omp for
        for (int i = 0; i < n; i++) {
            result[i] = data[i] + 10;
            printf("Thread %d: Phase 2, result[%d] = %d\n", omp_get_thread_num(), i, result[i]);
        }
    }

    printf("Processing completed with barrier synchronization.\n");
}

// ==================== 性能对比总结 ====================
// 推荐使用顺序（从高到低）：
// 1. reduction   - 最优性能，适用于规约操作
// 2. atomic      - 轻量级，适用于简单操作
// 3. lock        - 灵活但需谨慎使用
// 4. critical    - 通用但性能较差
// 5. ordered     - 性能最差，仅在必须保证顺序时使用

// ==================== default 子句的使用示例 ====================

// 示例1: default(shared) - 所有变量默认为共享
// 这是最简单的方式，但容易导致数据竞争，不推荐
void example_default_shared() {
    int       x       = 10;
    int       y       = 20;
    int       counter = 0; // 危险：所有线程共享，会有数据竞争
    const int c       = 5; // const 变量默认也是 shared

    printf("\n=== Example: default(shared) ===\n");
    printf("Initial: x=%d, y=%d, counter=%d\n", x, y, counter);

    // default(shared) 使所有未明确指定的变量都是 shared
    // 注意：这可能导致数据竞争！
#pragma omp parallel default(shared) num_threads(4)
    {
        int tid = omp_get_thread_num(); // 循环变量和这种局部变量默认是 private

        // 所有线程共享 x, y, counter
        // 这里会有数据竞争！
        counter++; // 危险操作！

        printf("Thread %d: x=%d, y=%d, counter=%d, const c=%d\n", tid, x, y, counter, c);
    }

    printf("After parallel: x=%d, y=%d, counter=%d (可能不是4，因为有数据竞争)\n", x, y, counter);
    printf("WARNING: 这个例子展示了不安全的使用方式！\n");
}

// 示例2: default(none) - 强制显式指定所有变量（最佳实践）
// 这是最推荐的方式，迫使程序员明确每个变量的共享属性
void example_default_none() {
    int       shared_data   = 100;
    int       result        = 0;
    const int n             = 10;
    int       private_value = 5;

    printf("\n=== Example: default(none) - Best Practice ===\n");
    printf("Initial: shared_data=%d, private_value=%d\n", shared_data, private_value);

    // default(none) 要求显式指定每个变量的属性
    // 这样可以避免意外的共享或私有化
#pragma omp parallel default(none) shared(shared_data, result, n) private(private_value) \
    num_threads(4)
    {
        int tid = omp_get_thread_num();

        // 每个线程有自己的 private_value 副本（未初始化）
        private_value = tid * 10;

        printf("Thread %d: shared_data=%d, private_value=%d, const n=%d\n", tid, shared_data,
               private_value, n);

// 安全地更新共享变量
#pragma omp atomic
        result += private_value;
    }

    printf("After parallel: result=%d, private_value=%d (不变)\n", result, private_value);
    printf("NOTE: default(none) 是最佳实践，强制明确声明变量属性\n");
}

// 示例3: 比较不同 default 子句的行为
void example_default_comparison() {
    printf("\n=== Example: Comparison of default clauses ===\n");

    int global_var = 42;

    // 情况1：不使用 default（编译器默认行为，通常是 default(shared)）
    printf("\n--- Without default clause (compiler default) ---\n");
#pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        printf("Thread %d: global_var=%d (shared by default)\n", tid, global_var);
    }

    // 情况2：显式使用 default(shared)
    printf("\n--- With default(shared) ---\n");
#pragma omp parallel default(shared) num_threads(2)
    {
        int tid = omp_get_thread_num();
        printf("Thread %d: global_var=%d (explicitly shared)\n", tid, global_var);
    }

    // 情况3：使用 default(none) 强制显式声明
    printf("\n--- With default(none) ---\n");
#pragma omp parallel default(none) shared(global_var) num_threads(2)
    {
        int tid = omp_get_thread_num();
        printf("Thread %d: global_var=%d (explicitly declared)\n", tid, global_var);
    }
}

// 示例4: default(none) 与 firstprivate
// firstprivate 会初始化每个线程的私有副本为原始值
void example_default_none_with_firstprivate() {
    int x           = 100;
    int y           = 200;
    int accumulator = 0;

    printf("\n=== Example: default(none) with firstprivate ===\n");
    printf("Initial: x=%d, y=%d\n", x, y);

    // firstprivate: 每个线程获得变量的私有副本，并初始化为原始值
    // private: 每个线程获得变量的私有副本，但不初始化
#pragma omp parallel default(none) firstprivate(x, y) shared(accumulator) num_threads(4)
    {
        int tid = omp_get_thread_num();

        // 每个线程的 x, y 都初始化为原始值
        x += tid * 10; // 修改私有副本
        y += tid * 20;

        printf("Thread %d: x=%d, y=%d\n", tid, x, y);

#pragma omp atomic
        accumulator += x + y;
    }

    printf("After parallel: x=%d, y=%d (不变), accumulator=%d\n", x, y, accumulator);
    printf("NOTE: firstprivate 会初始化私有副本，private 不会\n");
}

// 示例5: 常见错误和注意事项
void example_default_common_pitfalls() {
    printf("\n=== Example: Common Pitfalls with default clause ===\n");

    int        array[10]  = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int *      ptr        = array;
    static int static_var = 100; // 静态变量总是 shared

    printf("\n--- Pitfall 1: 指针变量的共享属性 ---\n");
    // 注意：指针本身可以是 private，但它指向的数据仍然是共享的
#pragma omp parallel default(none) shared(array, ptr) num_threads(2)
    {
        int tid = omp_get_thread_num();
        // ptr 是 shared 的，所有线程看到同一个指针
        // ptr 指向的数据（array）也是 shared 的
        if (tid == 0) {
            ptr[0] = 100; // 修改共享数据
        }
#pragma omp barrier
        printf("Thread %d: ptr[0]=%d\n", tid, ptr[0]);
    }

    printf("\n--- Pitfall 2: 静态变量总是 shared ---\n");
// static_var 总是 shared，在使用 default(none) 时需要显式声明
#pragma omp parallel default(none) shared(static_var) num_threads(2)
    {
        int tid = omp_get_thread_num();
        // 静态变量虽然总是 shared，但在 default(none) 时必须显式声明
#pragma omp atomic
        static_var += tid;
        printf("Thread %d: static_var=%d\n", tid, static_var);
    }

    printf("\n--- Pitfall 3: const 变量默认是 shared ---\n");
    const int const_val = 42;
#pragma omp parallel default(none) shared(const_val) num_threads(2)
    {
        int tid = omp_get_thread_num();
        // const 变量通常需要在 shared 子句中声明（使用 default(none) 时）
        printf("Thread %d: const_val=%d\n", tid, const_val);
    }
}

// 示例6: default 子句在嵌套并行区域中的使用
void example_default_nested_parallel() {
    int outer_var = 10;
    int inner_var = 20;

    printf("\n=== Example: default in nested parallel regions ===\n");
    printf("Initial: outer_var=%d, inner_var=%d\n", outer_var, inner_var);

    // 外层并行区域
#pragma omp parallel default(none) shared(outer_var, inner_var) num_threads(2)
    {
        int outer_tid = omp_get_thread_num();

        printf("Outer thread %d: outer_var=%d\n", outer_tid, outer_var);

        // 内层并行区域（嵌套）
        // 注意：需要设置 OMP_NESTED=true 或调用 omp_set_nested(1)
#pragma omp parallel default(none) shared(outer_var, inner_var, outer_tid) num_threads(2)
        {
            int inner_tid = omp_get_thread_num();
            printf("  Inner thread %d.%d: outer_var=%d, inner_var=%d\n", outer_tid, inner_tid,
                   outer_var, inner_var);
        }
    }

    printf("NOTE: 嵌套并行需要启用，每层都可以有自己的 default 子句\n");
}

// 示例7: default 与循环变量
void example_default_with_loop_variables() {
    int sum = 0;
    int n   = 10;

    printf("\n=== Example: default with loop variables ===\n");

    // 循环变量（如 for 循环的 i）默认是 private 的
    // 即使使用 default(shared)，循环变量也是 private
    // 注意：reduction 变量不能同时在 shared 子句中声明
#pragma omp parallel for default(none) shared(n) reduction(+ : sum)
    for (int i = 0; i < n; i++) {
        // i 自动是 private 的，不需要显式声明
        sum += i;
        printf("Thread %d processes i=%d\n", omp_get_thread_num(), i);
    }

    printf("Sum: %d (Expected: %d)\n", sum, n * (n - 1) / 2);
    printf("NOTE: for 循环的迭代变量自动是 private 的\n");
}

// ==================== default 子句使用总结 ====================
//
// 1. default(shared) - 所有变量默认共享
//    - 优点：简单，不需要显式声明
//    - 缺点：容易导致数据竞争，不推荐
//    - 使用场景：快速原型开发，但生产代码不推荐
//
// 2. default(none) - 强制显式声明（强烈推荐）
//    - 优点：强制程序员思考每个变量的共享属性，避免意外错误
//    - 缺点：需要更多代码，但这是好事
//    - 使用场景：所有生产代码都应该使用
//
// 3. default(private) 和 default(firstprivate)
//    - 注意：在 C/C++ 中，OpenMP 规范不支持这些选项
//    - 只有 default(shared) 和 default(none) 是标准的
//
// 重要规则：
// - 循环变量（如 for 循环的 i）总是 private 的
// - static 变量总是 shared 的
// - const 变量通常是 shared 的
// - 函数内部声明的局部变量，在并行区域外定义的，根据 default 子句决定
// - 指针本身可以是 private，但指向的数据可能是 shared
//
// 最佳实践：
// 1. 始终使用 default(none)
// 2. 显式声明每个变量为 shared, private, 或 firstprivate
// 3. 使用 reduction 子句处理规约操作
// 4. 使用 atomic 或 critical 保护共享变量的修改

} // namespace

int main() {
    printf("===== OpenMP Examples: Thread Management =====\n");
    // example_num_threads();
    // example_set_num_threads();

    printf("\n===== OpenMP Examples: Data Race Prevention Methods =====\n");

    // 运行所有避免数据竞争的示例
    // example_critical();        // 方法1：临界区
    // example_atomic();          // 方法2：原子操作
    // example_reduction();       // 方法3：规约（推荐）
    // example_lock();            // 方法4：显式锁
    // example_ordered();         // 方法5：有序执行
    // example_named_critical();  // 方法6：命名临界区
    // example_barrier();         // 方法7：屏障同步

    printf("\n===== OpenMP Examples: default Clause Usage =====\n");

    // 运行所有 default 子句的示例
    example_default_shared(); // 示例1：default(shared) - 不推荐
    example_default_none(); // 示例2：default(none) - 最佳实践
    example_default_comparison(); // 示例3：比较不同 default 子句
    example_default_none_with_firstprivate(); // 示例4：default(none) 与 firstprivate
    example_default_common_pitfalls(); // 示例5：常见错误和注意事项
    example_default_nested_parallel(); // 示例6：嵌套并行区域
    example_default_with_loop_variables(); // 示例7：循环变量

    printf("\n===== All examples completed =====\n");

    return 0;
}
