# C++ 内存池教学项目

## 📚 学习路径

本项目采用递进式教学设计，从基础到高级分为三个层级：

### 🟢 初级：基础内存管理 (basic/)
学习时间：2-3 天
- **01_raw_pointer_lifecycle.cpp** - 原始指针的生命周期管理
- **02_memory_leak_patterns.cpp** - 内存泄漏的常见模式
- **03_smart_pointer_usage.cpp** - 智能指针的正确使用场景

**学习目标**：
- 理解 new/delete 的工作原理
- 识别常见的内存泄漏模式
- 掌握 unique_ptr/shared_ptr/weak_ptr 的使用场景

### 🟡 中级：自定义内存管理 (intermediate/)
学习时间：4-5 天
- **04_fixed_size_pool** - 固定大小内存池实现
- **05_alignment_demo.cpp** - 对齐内存分配
- **06_free_list_algorithm.cpp** - 空闲列表算法详解

**学习目标**：
- 实现单链表管理的固定块池
- 理解内存对齐的重要性
- 掌握空闲列表的管理技巧

### 🔴 高级：生产级特性 (advanced/)
学习时间：5-7 天
- **07_paged_pool** - 分页式可变大小池
- **08_stack_allocator** - 栈式分配器
- **09_thread_safe_pool** - 线程安全实现
- **10_stl_allocator** - STL 容器集成

**学习目标**：
- 实现生产级内存池
- 掌握多线程内存管理
- 集成到 STL 容器

## 🔧 构建说明

```bash
# 构建所有示例
mkdir build && cd build
cmake ..
make

# 运行特定示例
./build/bin/memory_pool/basic_raw_pointer_lifecycle

# 使用 ASan 检测内存问题
cmake -DENABLE_ASAN=ON ..
make
./build/bin/memory_pool/basic_memory_leak_patterns

# 使用 Valgrind
valgrind --leak-check=full ./build/bin/memory_pool/basic_memory_leak_patterns