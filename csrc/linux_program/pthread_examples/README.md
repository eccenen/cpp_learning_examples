# pthread 编程示例

本目录包含全面的 Linux pthread 编程示例，展示了各种线程编程技术和最佳实践。

## 目录结构

```
pthread_examples/
├── pthread_comprehensive_example.cpp  # 纯pthread综合示例（推荐学习）
├── basic_thread_ops.cpp              # 基础线程操作
├── sync_mechanisms.cpp               # 同步机制演示
├── sync_primitives.cpp/h             # 同步原语封装
├── advanced_features.cpp             # 高级特性
├── thread_pool.cpp/h                 # 线程池实现
├── thread_pool_demo.cpp              # 线程池演示
└── CMakeLists.txt                    # 构建配置
```

## 编译

从项目根目录运行：

```bash
./compile.sh
```

或者手动编译：

```bash
mkdir -p build && cd build
cmake ..
make
```

## 运行示例

编译后，可执行文件位于 `build/csrc/linux_program/pthread_examples/` 目录下：

### 1. pthread 综合示例（推荐）

```bash
./build/csrc/linux_program/pthread_examples/pthread_comprehensive_example
```

**包含的内容：**
- ✅ 基础线程创建、等待和分离
- ✅ 互斥锁（Mutex）使用
- ✅ 条件变量（生产者-消费者模式）
- ✅ 读写锁（Read-Write Lock）
- ✅ 信号量（Semaphore）
- ✅ 线程属性设置
- ✅ 线程取消机制
- ✅ 线程局部存储（TLS）
- ✅ 常见错误示例和最佳实践

**特点：**
- 完全使用 pthread API，不依赖 C++ 标准库的 thread
- 符合 Google C++ 代码规范
- 充分的中文注释
- 包含常见错误说明

### 2. 其他示例

```bash
# 基础线程操作
./build/csrc/linux_program/pthread_examples/basic_thread_ops

# 同步机制（使用C++封装）
./build/csrc/linux_program/pthread_examples/sync_mechanisms

# 高级特性
./build/csrc/linux_program/pthread_examples/advanced_features

# 线程池演示
./build/csrc/linux_program/pthread_examples/thread_pool_demo
```

## 示例详解

### pthread_comprehensive_example.cpp

这是最全面的纯 pthread 示例，包含 10 个主要部分：

#### 示例 1: 基础线程操作
- 线程创建 `pthread_create()`
- 线程等待 `pthread_join()`
- 参数传递和返回值获取

#### 示例 2: 线程分离
- 使用 `pthread_detach()` 分离线程
- 分离线程的自动资源清理

#### 示例 3: 互斥锁
- `pthread_mutex_lock()` / `pthread_mutex_unlock()`
- 防止竞态条件
- 保护共享数据

#### 示例 4: 条件变量
- 生产者-消费者模式实现
- `pthread_cond_wait()` / `pthread_cond_signal()`
- 与互斥锁配合使用

#### 示例 5: 读写锁
- `pthread_rwlock_rdlock()` - 读锁
- `pthread_rwlock_wrlock()` - 写锁
- 多读者单写者模式

#### 示例 6: 信号量
- `sem_init()` / `sem_wait()` / `sem_post()`
- 控制并发访问数量
- 资源计数

#### 示例 7: 线程属性
- 栈大小设置
- 分离状态设置
- 调度策略查询

#### 示例 8: 线程取消
- `pthread_cancel()` 取消线程
- `pthread_testcancel()` 设置取消点
- 取消状态和类型设置

#### 示例 9: 线程局部存储
- `pthread_key_create()` 创建 TLS 键
- `pthread_setspecific()` / `pthread_getspecific()`
- TLS 析构函数

#### 示例 10: 常见错误
- 忘记初始化互斥锁
- 死锁情况
- 忘记解锁
- 条件变量使用 if 而不是 while
- 资源泄漏
- 数据竞争

## 常见陷阱和最佳实践

### ✅ 最佳实践

1. **总是检查返回值**
   ```cpp
   int rc = pthread_create(&thread, nullptr, func, arg);
   if (rc != 0) {
       // 处理错误
   }
   ```

2. **使用 while 而不是 if 检查条件变量**
   ```cpp
   while (!condition) {
       pthread_cond_wait(&cond, &mutex);
   }
   ```

3. **确保每个线程都被 join 或 detach**
   ```cpp
   pthread_detach(thread);  // 或
   pthread_join(thread, nullptr);
   ```

4. **临界区尽量小**
   ```cpp
   pthread_mutex_lock(&mutex);
   // 只在这里访问共享数据
   pthread_mutex_unlock(&mutex);
   ```

5. **清理资源**
   ```cpp
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&cond);
   sem_destroy(&sem);
   ```

### ❌ 常见错误

1. **忘记初始化同步原语**
2. **死锁 - 重复加锁或锁顺序不一致**
3. **忘记解锁互斥锁**
4. **在持有锁时执行耗时操作**
5. **访问已销毁线程的栈数据**
6. **不检查函数返回值**

## 编译选项说明

```cmake
-std=c++17      # 使用 C++17 标准
-Wall -Wextra   # 启用所有警告
-O2             # 优化级别 2
-g              # 包含调试信息
-lpthread       # 链接 pthread 库
```

## 学习路径建议

1. 首先运行 `pthread_comprehensive_example` 完整体验
2. 阅读源代码中的中文注释
3. 尝试修改参数观察不同行为
4. 实验故意引入错误，理解常见陷阱
5. 参考代码实现自己的多线程程序

## 相关资源

- POSIX Threads Programming: https://computing.llnl.gov/tutorials/pthreads/
- pthread man pages: `man pthread_create`
- Google C++ Style Guide: https://google.github.io/styleguide/cppguide.html

## 注意事项

- 某些示例（如线程取消、实时调度）可能需要特定权限
- 运行时间较长的示例可以使用 Ctrl+C 中断
- 建议在 Linux 环境下运行这些示例