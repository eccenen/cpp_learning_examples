# 可变参数快速参考卡片

## C++ 可变参数速查表

### 基础语法

```cpp
// 声明
template <typename... Args>        // 类型参数包
void func(Args... args);           // 函数参数包

template <int... Nums>             // 非类型参数包
constexpr int sum = (Nums + ...);

// 获取参数个数
sizeof...(Args)   // 类型参数包大小
sizeof...(args)   // 函数参数包大小
```

### 展开方式一览

| 方式 | 语法 | C++版本 | 推荐度 |
|------|------|---------|--------|
| 递归展开 | `func(first, rest...)` | C++11 | ⭐⭐ |
| 逗号展开 | `{(func(args), 0)...}` | C++11 | ⭐⭐ |
| 折叠表达式 | `(... + args)` | C++17 | ⭐⭐⭐⭐⭐ |
| 初始化列表 | `{args...}` | C++11 | ⭐⭐⭐⭐ |

### 折叠表达式速查 (C++17)

```cpp
// 一元折叠
(... + args)      // 左折叠: ((a1 + a2) + a3) + ...
(args + ...)      // 右折叠: a1 + (a2 + (a3 + ...))

// 二元折叠
(init + ... + args)   // 左折叠: ((init + a1) + a2) + ...
(args + ... + init)   // 右折叠: a1 + (a2 + (... + init))

// 常用运算符
(... + args)          // 求和
(... && args)         // 全部为真
(... || args)         // 任一为真
((cout << args), ...) // 依次输出
```

### 常见模式

```cpp
// 1. 打印所有参数
template <typename... Args>
void print(Args&&... args) {
    ((std::cout << std::forward<Args>(args) << " "), ...);
}

// 2. 求和
template <typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

// 3. 全部为真
template <typename... Args>
bool all_true(Args... args) {
    return (... && args);
}

// 4. 完美转发
template <typename Func, typename... Args>
decltype(auto) forward_call(Func&& func, Args&&... args) {
    return std::forward<Func>(func)(std::forward<Args>(args)...);
}

// 5. 应用到每个元素
template <typename Func, typename... Args>
void for_each(Func&& func, Args&&... args) {
    (func(std::forward<Args>(args)), ...);
}

// 6. 参数包转容器
template <typename... Args>
auto to_vector(Args... args) {
    return std::vector{args...};
}

// 7. 类型约束
template <typename... Args>
    requires (std::integral<Args> && ...)
auto sum_ints(Args... args) {
    return (args + ...);
}
```

---

## C 可变参数速查表

### 基础宏

```c
#include <stdarg.h>

va_list ap;              // 声明参数列表
va_start(ap, last);      // 初始化，last 是最后一个固定参数
va_arg(ap, type);        // 获取下一个参数
va_end(ap);              // 清理
va_copy(dest, src);      // 复制参数列表
```

### 基本模板

```c
// 1. 带计数的可变参数
int sum(int count, ...) {
    va_list args;
    va_start(args, count);
    
    int result = 0;
    for (int i = 0; i < count; ++i) {
        result += va_arg(args, int);
    }
    
    va_end(args);
    return result;
}

// 2. NULL 终止的可变参数
char* concat(const char* first, ...) {
    va_list args;
    va_start(args, first);
    
    const char* str;
    while ((str = va_arg(args, const char*)) != NULL) {
        // 处理 str
    }
    
    va_end(args);
}

// 3. 格式字符串驱动
void custom_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    for (const char* p = format; *p; ++p) {
        if (*p == '%') {
            switch (*++p) {
                case 'd': printf("%d", va_arg(args, int)); break;
                case 's': printf("%s", va_arg(args, char*)); break;
                // ...
            }
        } else {
            putchar(*p);
        }
    }
    
    va_end(args);
}
```

### 类型提升规则

| 传入类型 | 使用 va_arg 时的类型 |
|---------|---------------------|
| `char` | `int` |
| `short` | `int` |
| `float` | `double` |
| `int` | `int` |
| `double` | `double` |
| `T*` | `T*` |

### 常见陷阱

```c
// ❌ 错误：忘记 va_end
void bad_func(int count, ...) {
    va_list args;
    va_start(args, count);
    // ... 使用 args ...
    // 忘记调用 va_end(args)!
}

// ❌ 错误：类型不匹配
void bad_types(int count, ...) {
    va_list args;
    va_start(args, count);
    
    double d = 3.14;  // 实际传入
    int i = va_arg(args, int);  // 期望 int → 未定义行为！
    
    va_end(args);
}

// ❌ 错误：调用次数过多
void bad_count(int count, ...) {
    va_list args;
    va_start(args, count);
    
    for (int i = 0; i < count + 1; ++i) {  // 多调用一次!
        va_arg(args, int);
    }
    
    va_end(args);
}

// ✅ 正确：使用 va_copy
void correct_copy(int count, ...) {
    va_list args, args_copy;
    va_start(args, count);
    va_copy(args_copy, args);  // 复制
    
    // 使用 args
    // 使用 args_copy
    
    va_end(args_copy);
    va_end(args);
}
```

---

## 对比选择

### 何时使用 C++ 可变参数模板

✅ 需要类型安全  
✅ 性能关键代码  
✅ 编译期计算  
✅ 仅 C++ 项目  
✅ 复杂类型操作  

### 何时使用 C 可变参数

✅ 需要 C 兼容性  
✅ 简单的变长参数  
✅ printf 风格接口  
✅ 动态参数数量  
✅ 学习曲线要求低  

---

## 实用代码片段

### C++ 实用工具

```cpp
// 1. 打印任意类型
template <typename... Args>
void log(Args&&... args) {
    ((std::cout << std::forward<Args>(args) << " "), ...);
    std::cout << '\n';
}

// 2. 创建 tuple
template <typename... Args>
auto make_tuple(Args&&... args) {
    return std::tuple<Args...>(std::forward<Args>(args)...);
}

// 3. 类型检查
template <typename T, typename... Args>
constexpr bool are_all_same_v = (std::is_same_v<T, Args> && ...);

// 4. 任意类型转字符串
template <typename... Args>
std::string to_string(Args&&... args) {
    std::ostringstream oss;
    ((oss << std::forward<Args>(args) << " "), ...);
    return oss.str();
}

// 5. 条件调用
template <typename Func, typename... Args>
void call_if_all_true(Func&& func, Args... conditions) {
    if ((... && conditions)) {
        std::forward<Func>(func)();
    }
}
```

### C 实用工具

```c
// 1. 安全求和
int safe_sum(int count, ...) {
    if (count < 0) return 0;
    
    va_list args;
    va_start(args, count);
    
    int sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += va_arg(args, int);
    }
    
    va_end(args);
    return sum;
}

// 2. 字符串拼接
char* str_concat(const char* first, ...) {
    va_list args;
    va_start(args, first);
    
    // 计算总长度
    size_t total = strlen(first);
    va_list args_copy;
    va_copy(args_copy, args);
    
    const char* s;
    while ((s = va_arg(args_copy, const char*)) != NULL) {
        total += strlen(s);
    }
    va_end(args_copy);
    
    // 分配内存并拼接
    char* result = malloc(total + 1);
    strcpy(result, first);
    
    while ((s = va_arg(args, const char*)) != NULL) {
        strcat(result, s);
    }
    
    va_end(args);
    return result;
}

// 3. 数组平均值
double array_average(int count, ...) {
    if (count <= 0) return 0.0;
    
    va_list args;
    va_start(args, count);
    
    double sum = 0.0;
    for (int i = 0; i < count; ++i) {
        sum += va_arg(args, double);
    }
    
    va_end(args);
    return sum / count;
}
```

---

## 调试技巧

### C++ 调试

```cpp
// 1. 打印参数类型
template <typename... Args>
void print_types(Args&&... args) {
    ((std::cout << typeid(args).name() << " "), ...);
    std::cout << '\n';
}

// 2. 编译期检查参数个数
template <typename... Args>
void check_args(Args... args) {
    static_assert(sizeof...(args) > 0, "至少需要一个参数");
    static_assert(sizeof...(args) <= 10, "参数过多");
}

// 3. 运行时参数信息
template <typename... Args>
void debug_args(const char* name, Args&&... args) {
    std::cout << name << ": "
              << "count=" << sizeof...(args) << ", "
              << "types=";
    print_types(std::forward<Args>(args)...);
}
```

### C 调试

```c
// 1. 参数计数验证
void debug_sum(int count, ...) {
    printf("Expected count: %d\n", count);
    
    va_list args;
    va_start(args, count);
    
    for (int i = 0; i < count; ++i) {
        int val = va_arg(args, int);
        printf("  arg[%d] = %d\n", i, val);
    }
    
    va_end(args);
}

// 2. 类型标记调试
void debug_mixed(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("Format: %s\n", format);
    for (const char* p = format; *p; ++p) {
        if (*p == '%') {
            printf("  Found type specifier: %c\n", *++p);
        }
    }
    
    va_end(args);
}
```

---

## 性能考虑

### C++ 性能优势

- ✅ **零开销抽象**：编译期展开，无运行时开销
- ✅ **内联优化**：编译器可完全内联
- ✅ **类型特化**：针对每个类型优化

### C 性能特点

- ⚠️ **运行时遍历**：必须逐个提取参数
- ⚠️ **栈操作开销**：va_start/va_arg 有开销
- ✅ **简单实现**：编译速度快

---

## 总结

| 特性 | C++ 可变参数 | C 可变参数 |
|------|-------------|-----------|
| **类型安全** | ✅ 编译期 | ❌ 无检查 |
| **性能** | ✅ 零开销 | ⚠️ 有开销 |
| **易用性** | ⚠️ 复杂 | ✅ 简单 |
| **灵活性** | ✅ 极高 | ⚠️ 受限 |
| **C 兼容** | ❌ 不兼容 | ✅ 兼容 |
| **学习曲线** | ⚠️ 陡峭 | ✅ 平缓 |

**推荐原则**：
- 新 C++ 项目：优先使用可变参数模板
- 需要 C 兼容：使用 C 可变参数
- 性能关键：使用折叠表达式
- 简单场景：根据团队熟悉度选择
