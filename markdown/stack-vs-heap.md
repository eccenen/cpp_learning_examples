# 在堆上创建 vs 在栈上创建：示例与实践

# 1. 如何在 C++ 中强制一个类只能在堆上创建对象，或只能在栈上创建对象？
## 在堆上创建（Heap-only）
1) 把构造函数设为私有，并提供静态工厂返回智能指针`std::unique_ptr`。这样外部无法直接调用构造函数在栈上分配，也无法拷贝。
 - 优点：可以控制对象的生命周期，强制使用智能指针，防止栈上分配导致的语义错误。
 - 缺点：需要额外的分配开销（堆分配），且在某些场景更难以在编译期保证性能。

2) 把析构函数设置为私有。
 - 优点：无需提供额外的，用于构造对象的工厂函数
 - 缺点：需要提供额外的，释放对象内存的函数，且在对象生命周期结束时，需要手动调用该函数进行释放。
 ### 示例源码
 [csrc/heap_only.h](../csrc/heap_only.h)

## 构建与测试

项目已提供 `compile.sh`（或 `build.sh`）脚本，会自动处理：

- vcpkg toolchain 自动检测（优先）；
- 若找不到 vcpkg，会使用 FetchContent 回退下载 doctest 用于测试；
- 会生成 `compile_commands.json` 并在仓库根创建符号链接，便于 clangd 使用。

快速构建：

```bash
# 指定 VCPKG_PATH（可选）
VCPKG_PATH=/home/you/software/vcpkg ./compile.sh

# 或者明确指定 toolchain
./compile.sh --toolchain /home/you/software/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## 进一步阅读

- C++ 对象模型与动态/静态分配的性能与语义差异。
- smart pointer 使用模式（unique_ptr/shared_ptr）与工厂模式。

如果你希望我把这篇文档加到 README 的示例列表或生成 HTML 文档，请告诉我我会继续操作。
