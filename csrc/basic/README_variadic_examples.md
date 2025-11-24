# 可变参数示例详解

## 文件说明

`variadic_examples.cpp` - C++ 和 C 语言可变参数的全面示例

本文件提供了可变参数编程的完整学习材料，涵盖所有常用场景和高级技巧。

---

## 目录

### 第一部分：C++ 可变参数模板

1. **基础递归展开** - 最经典的可变参数模板用法
2. **sizeof... 运算符** - 获取参数包的大小
3. **逗号表达式展开** - 利用初始化列表展开参数包
4. **折叠表达式 (C++17)** - 最简洁的参数包展开方式
   - 一元右折叠
   - 一元左折叠
   - 二元折叠
   - 逻辑运算折叠
5. **参数包展开到容器** - 构造 vector 等容器
6. **完美转发** - 保持参数的值类别
7. **类型萃取和 SFINAE** - 编译期类型检查
8. **可变参数模板类** - Tuple 和 Variant 风格
9. **参数包索引访问** - 获取第 N 个参数
10. **可变参数 Lambda** - 泛型 lambda 表达式
11. **初始化列表结合** - 参数包与 initializer_list
12. **编译期计算** - 递归模板实例化
13. **混合参数** - 类型参数和非类型参数

### 第二部分：C 语言可变参数

1. **基本用法** - va_start, va_arg, va_end
2. **处理不同类型** - 格式字符串指导类型解析
3. **可变参数算法** - 最大值、最小值等
4. **va_copy** - 复制参数列表
5. **字符串操作** - NULL 终止符标记
6. **数组参数** - 处理多个数组
7. **嵌套调用** - va_list 作为参数
8. **类型安全警告** - 常见陷阱

---

## 编译和运行

```bash
# 编译
cmake --build build --target variadic_examples

# 运行
./build/bin/basic/variadic_examples
```

---

## C++ 可变参数核心概念

### 1. 参数包 (Parameter Pack)

```cpp
template <typename... Args>  // Args 是类型参数包
void func(Args... args);     // args 是函数参数包
```

### 2. 参数包展开 (Pack Expansion)

```cpp
// 展开方式：pattern...
func(args...);              // 展开为: func(arg1, arg2, arg3)
func(process(args)...);     // 展开为: func(process(arg1), process(arg2), ...)
```

### 3. 折叠表达式 (C++17)

| 表达式 | 展开结果 |
|--------|---------|
| `(... op args)` | `((arg1 op arg2) op arg3) op ...` (左折叠) |
| `(args op ...)` | `arg1 op (arg2 op (arg3 op ...))` (右折叠) |
| `(init op ... op args)` | `((init op arg1) op arg2) op ...` (二元左折叠) |
| `(args op ... op init)` | `arg1 op (arg2 op (... op init))` (二元右折叠) |

支持的运算符：`+`, `-`, `*`, `/`, `%`, `&`, `|`, `^`, `<<`, `>>`, `&&`, `||`, `,`, `.*`, `->*`

### 4. 递归展开模式

```cpp
// 基本情况（递归终止）
void print() {}

// 递归情况
template <typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << " ";
    print(rest...);  // 递归
}
```

### 5. 完美转发

```cpp
template <typename... Args>
auto wrapper(Args&&... args) {
    return target(std::forward<Args>(args)...);
}
```

---

## C 语言可变参数核心概念

### 1. 基本宏

- `va_list` - 参数列表类型
- `va_start(ap, last)` - 初始化参数列表
- `va_arg(ap, type)` - 获取下一个参数
- `va_end(ap)` - 清理参数列表
- `va_copy(dest, src)` - 复制参数列表

### 2. 类型提升规则

调用可变参数函数时，编译器会自动进行默认参数提升：

- `char` 和 `short` → `int`
- `float` → `double`

因此，使用 `va_arg` 时必须使用提升后的类型：

```c
va_arg(args, int)     // 而不是 char 或 short
va_arg(args, double)  // 而不是 float
```

### 3. 终止标记

C 语言可变参数无法自动知道参数个数，常见解决方案：

- **参数个数作为第一个参数**：`func(int count, ...)`
- **格式字符串**：`printf(const char* format, ...)`
- **特殊终止值**：`NULL`、`-1` 等
- **哨兵值**：如字符串数组的 `nullptr`

### 4. 类型安全问题

C 可变参数**不进行类型检查**！必须通过约定确保类型正确：

```c
// 错误示例
int sum = va_arg(args, int);  // 期望 int
// 但实际传入的是 double，导致未定义行为！
```

---

## 使用场景对比

| 特性 | C++ 可变参数模板 | C 可变参数 |
|------|-----------------|-----------|
| 类型安全 | ✅ 编译期检查 | ❌ 运行时无检查 |
| 性能 | ✅ 零开销抽象 | ⚠️ 运行时遍历 |
| 灵活性 | ✅ 任意类型 | ⚠️ 需要类型提升 |
| 学习曲线 | ⚠️ 较陡峭 | ✅ 简单直接 |
| C 兼容性 | ❌ 仅 C++ | ✅ C/C++ 通用 |

---

## 最佳实践

### C++ 可变参数

1. **优先使用折叠表达式** (C++17+)
   ```cpp
   // 推荐
   return (args + ...);
   
   // 避免不必要的递归
   return sum_recursive(args...);
   ```

2. **使用 SFINAE 或 Concepts 约束类型**
   ```cpp
   template <typename... Args>
   requires (std::integral<Args> && ...)
   auto sum(Args... args);
   ```

3. **完美转发保持值类别**
   ```cpp
   template <typename... Args>
   void forward_call(Args&&... args) {
       target(std::forward<Args>(args)...);
   }
   ```

4. **索引访问使用 std::get 或 std::tuple**
   ```cpp
   auto tup = std::make_tuple(args...);
   auto first = std::get<0>(tup);
   ```

### C 语言可变参数

1. **总是明确参数个数或终止条件**
   ```c
   int sum(int count, ...);           // 明确个数
   char* concat(const char* first, ...); // NULL 终止
   ```

2. **注意类型提升**
   ```c
   va_arg(args, int)    // 而非 char
   va_arg(args, double) // 而非 float
   ```

3. **使用 va_copy 复制参数列表**
   ```c
   va_list copy;
   va_copy(copy, original);
   // 使用 copy
   va_end(copy);
   ```

4. **提供类型安全的包装**
   ```c
   // 内部使用可变参数，外部提供类型安全接口
   int sum_ints(int count, int* values) {
       // 内部调用可变参数函数
   }
   ```

---

## 常见陷阱

### C++ 陷阱

1. **忘记 `...` 展开**
   ```cpp
   func(args);      // 错误：传递参数包本身
   func(args...);   // 正确：展开参数包
   ```

2. **递归无终止条件**
   ```cpp
   // 需要提供基本情况
   void print() {}  // 递归终止
   template <typename T, typename... Args>
   void print(T first, Args... rest);
   ```

3. **完美转发丢失引用**
   ```cpp
   forward(args...);                // 错误
   forward(std::forward<Args>(args)...);  // 正确
   ```

### C 陷阱

1. **忘记 va_end**
   ```c
   va_list args;
   va_start(args, last);
   // ... 使用 args ...
   va_end(args);  // 必须调用！
   ```

2. **类型不匹配**
   ```c
   double d = 3.14;
   func(1, d);  // 传入 double
   // 函数内部
   int i = va_arg(args, int);  // 错误！应该是 double
   ```

3. **va_arg 调用次数错误**
   ```c
   // 调用次数超过实际参数数量 → 未定义行为
   for (int i = 0; i < count + 1; ++i) {  // 错误：多调用一次
       va_arg(args, int);
   }
   ```

---

## 进阶主题

### 1. 可变参数与 SFINAE

```cpp
// 仅接受所有相同类型的参数
template <typename T, typename... Args>
std::enable_if_t<(std::is_same_v<T, Args> && ...), T>
sum(T first, Args... rest) {
    return (first + ... + rest);
}
```

### 2. constexpr 可变参数

```cpp
template <int... Values>
constexpr int sum = (Values + ...);

static_assert(sum<1, 2, 3, 4> == 10);
```

### 3. 可变参数与 CRTP

```cpp
template <typename... Mixins>
class Combined : public Mixins... {
public:
    using Mixins::func...;  // C++17: 展开 using 声明
};
```

### 4. 编译期字符串拼接

```cpp
template <char... Chars>
struct String {
    static constexpr char value[] = {Chars..., '\0'};
};
```

---

## 参考资料

- C++17 标准：折叠表达式 ([expr.prim.fold])
- C++11 标准：可变参数模板 ([temp.variadic])
- C99 标准：可变参数 ([stdarg.h])
- Google C++ Style Guide
- Effective Modern C++ (Scott Meyers)

---

## 总结

本示例文件通过 **13 种 C++ 技术** 和 **8 种 C 技术** 全面覆盖了可变参数编程：

✅ **C++ 优势**：类型安全、零开销、编译期计算  
✅ **C 优势**：简单直接、跨语言兼容  
✅ **实践价值**：理解现代 C++ 库的设计模式  
✅ **代码质量**：符合 Google 代码规范，注释详尽

通过学习本示例，你将掌握：
- 可变参数的所有展开技巧
- 类型安全的可变参数设计
- 编译期和运行时的权衡
- 实际项目中的应用场景
