# No Main Executable Example

## 概述

本示例演示如何在主源文件（`main.cpp`）中**不编写 main 函数**，但通过链接另一个提供 main 函数的源文件（`main_provider.cpp`）来生成可执行文件。这种技术在测试框架（如 Google Test）中被广泛使用。

## 文件说明

### 1. `main.cpp` - 用户代码（不包含 main 函数）

- **特点**：此文件中没有 `main()` 函数
- **功能**：
  - 定义测试用例和业务逻辑
  - 实现 `RunUserTests()` 函数，由 `main_provider.cpp` 调用
  - 使用自动注册机制（类似 gtest 的 `TEST` 宏）注册测试用例

### 2. `main_provider.cpp` - Main 函数提供者

- **特点**：提供 `main()` 函数入口
- **功能**：
  - 声明 `extern int RunUserTests()`（由用户代码实现）
  - 在 `main()` 中调用用户的测试逻辑
  - 处理测试框架的初始化和清理工作

### 3. `CMakeLists.txt` - 构建配置

- 将两个源文件链接成一个可执行文件：
  ```cmake
  add_executable(techniques_no_main_executable
      main.cpp           # 用户代码（不含 main）
      main_provider.cpp  # 提供 main 函数
  )
  ```

## 工作原理

1. **链接阶段**：
   - `main.cpp` 编译成目标文件，其中包含 `RunUserTests()` 的定义
   - `main_provider.cpp` 编译成目标文件，其中包含 `main()` 的定义
   - 链接器将两个目标文件链接成可执行文件

2. **符号解析**：
   - `main_provider.cpp` 中的 `extern int RunUserTests()` 声明告诉编译器该函数在别处定义
   - 链接时，链接器在 `main.cpp` 的目标文件中找到 `RunUserTests()` 的实现
   - 最终生成的可执行文件包含完整的符号引用

3. **执行流程**：
   ```
   程序启动 → main() [来自 main_provider.cpp]
            → RunUserTests() [来自 main.cpp]
            → 执行所有注册的测试用例
            → 返回结果
   ```

## 类比 Google Test

这个示例的设计理念与 Google Test 类似：

```cpp
// 使用 gtest 时，你只需要写：
TEST(TestSuite, TestCase) {
    EXPECT_EQ(2 + 2, 4);
}

// 不需要写 main 函数
// 因为 gtest 库已经提供了 main 函数
// 链接时：你的测试文件 + libgtest_main.a → 可执行文件
```

在本示例中：
```cpp
// main.cpp 中只需写：
TestRegistrar test("TestName", []() -> bool {
    return (2 + 2 == 4);
});

// 不需要写 main 函数
// 因为 main_provider.cpp 已经提供了
// 链接时：main.cpp + main_provider.cpp → 可执行文件
```

## 构建和运行

```bash
# 构建
cd /path/to/cpp-qa-lab
./compile.sh

# 运行
./build/bin/techniques/techniques_no_main_executable
```

## 输出示例

```
=== Custom Test Framework ===
Starting test execution...

Discovered 3 test case(s)

[TEST] BasicMath
  Running: BasicMath
    2 + 2 = 4, Expected: 4, Result: PASS

[TEST] StringOps
  Running: StringOps
    String concatenation: 'Hello World', Result: PASS

[TEST] VectorTest
  Running: VectorTest
    Vector size: 5, vec[2]: 3, Result: PASS

Summary: 3 passed, 0 failed

=== Test Execution Completed ===
Exit code: 0
```

## 扩展思路

1. **创建静态库**：可以将 `main_provider.cpp` 编译成静态库（`.a`），然后多个测试文件都链接这个库
2. **参数解析**：在 `main_provider.cpp` 中添加命令行参数解析，控制测试行为
3. **测试发现**：实现更复杂的测试注册和发现机制
4. **报告生成**：添加 XML/JSON 格式的测试报告输出

## 关键要点

✅ `main.cpp` 中**没有** `main()` 函数  
✅ 通过 `extern` 声明实现跨文件函数调用  
✅ 链接器负责解析符号引用  
✅ 这是测试框架的常见设计模式  
