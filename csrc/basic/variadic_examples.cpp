// ============================================================================
// Variadic Arguments Examples - C++ and C Comprehensive Guide
// ============================================================================
// 本文件展示了C++可变参数和C语言可变参数的所有使用情况
// 包括：
//   - C++ 可变参数模板（Variadic Templates）
//   - C++ 折叠表达式（Fold Expressions, C++17）
//   - C++ 参数包展开的各种方式
//   - C 语言可变参数（stdarg.h）
//
// 使用 fmt 库进行格式化输出，符合 Google C++ 代码风格。
// ============================================================================

#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <cstdarg> // C语言可变参数
#include <initializer_list>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// ============================================================================
// 第一部分：C++ 可变参数模板 (Variadic Templates)
// ============================================================================

namespace cpp_variadic {

// ----------------------------------------------------------------------------
// 1.1 基础递归展开 - 最经典的可变参数模板用法
// ----------------------------------------------------------------------------

// 递归终止条件：当参数包为空时调用此函数
void PrintRecursive() {
    fmt::print("\n"); // 结束打印
}

// 递归展开：每次处理一个参数，然后递归处理剩余参数
template <typename T, typename... Args> void PrintRecursive(T first, Args... rest) {
    fmt::print("{} ", first);
    PrintRecursive(rest...); // 递归调用，处理剩余参数
}

// ----------------------------------------------------------------------------
// 1.2 使用 sizeof... 获取参数个数
// ----------------------------------------------------------------------------

template <typename... Args> void PrintCount(Args... args) {
    fmt::print("参数个数: {}\n", sizeof...(args));
    fmt::print("参数类型个数: {}\n", sizeof...(Args));
}

// ----------------------------------------------------------------------------
// 1.3 逗号表达式展开 - 利用初始化列表强制求值顺序
// ----------------------------------------------------------------------------

// 辅助函数：打印单个值并返回0
template <typename T> int PrintSingle(T && value) {
    fmt::print("{} ", std::forward<T>(value));
    return 0;
}

// 使用逗号表达式在初始化列表中展开参数包
template <typename... Args> void PrintCommaExpansion(Args &&... args) {
    // 利用初始化列表的求值顺序（从左到右）
    // (PrintSingle(args), 0) 先调用PrintSingle，再取值0
    int dummy[] = { (PrintSingle(std::forward<Args>(args)), 0)... };
    (void) dummy; // 避免未使用变量警告
    fmt::print("\n");
}

// ----------------------------------------------------------------------------
// 1.4 折叠表达式 (C++17) - 最简洁的展开方式
// ----------------------------------------------------------------------------

// 1.4.1 一元右折叠：(args + ...)
template <typename... Args> auto SumFoldRight(Args... args) {
    return (args + ...); // 展开为: arg1 + (arg2 + (arg3 + arg4))
}

// 1.4.2 一元左折叠：(... + args)
template <typename... Args> auto SumFoldLeft(Args... args) {
    return (... + args); // 展开为: ((arg1 + arg2) + arg3) + arg4
}

// 1.4.3 二元右折叠：(args + ... + init)
template <typename... Args> auto SumWithInit(Args... args) {
    return (args + ... + 0); // 有初始值，避免空参数包错误
}

// 1.4.4 折叠表达式用于打印
template <typename... Args> void PrintFold(Args &&... args) {
    // 使用逗号运算符折叠
    ((fmt::print("{} ", std::forward<Args>(args))), ...);
    fmt::print("\n");
}

// 1.4.5 折叠表达式用于逻辑运算
template <typename... Args> bool AllTrue(Args... args) {
    return (... && args); // 左折叠：((arg1 && arg2) && arg3) && arg4
}

template <typename... Args> bool AnyTrue(Args... args) {
    return (... || args); // 左折叠：((arg1 || arg2) || arg3) || arg4
}

// ----------------------------------------------------------------------------
// 1.5 参数包展开到容器
// ----------------------------------------------------------------------------

// 1.5.1 展开到 vector
template <typename... Args> std::vector<int> PackToVector(Args... args) {
    return { args... }; // 列表初始化展开
}

// 1.5.2 展开并应用函数
template <typename Func, typename... Args> void ApplyToEach(Func func, Args &&... args) {
    // 使用折叠表达式依次调用函数
    (func(std::forward<Args>(args)), ...);
}

// ----------------------------------------------------------------------------
// 1.6 完美转发 (Perfect Forwarding)
// ----------------------------------------------------------------------------

// 包装函数：完美转发参数到目标函数
template <typename Func, typename... Args> decltype(auto) Invoke(Func && func, Args &&... args) {
    fmt::print("调用函数，参数个数: {}\n", sizeof...(args));
    return std::forward<Func>(func)(std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
// 1.7 类型萃取和SFINAE
// ----------------------------------------------------------------------------

// 1.7.1 检查所有类型是否相同
template <typename T, typename... Args>
struct AreAllSame : std::conjunction<std::is_same<T, Args>...> {};

// 1.7.2 仅接受整数类型的可变参数函数
template <typename... Args,
          typename = std::enable_if_t<std::conjunction_v<std::is_integral<Args>...>>>
int SumIntegers(Args... args) {
    return (... + args);
}

// ----------------------------------------------------------------------------
// 1.8 可变参数模板类
// ----------------------------------------------------------------------------

// 1.8.1 Tuple 的简化实现（递归继承）
template <typename... Types> class SimpleTuple;

// 特化：空元组
template <> class SimpleTuple<> {};

// 递归定义：每个元组存储一个值并继承剩余类型的元组
template <typename T, typename... Rest>
class SimpleTuple<T, Rest...> : private SimpleTuple<Rest...> {
  public:
    SimpleTuple(T value, Rest... rest) : SimpleTuple<Rest...>(rest...), value_(value) {}

    T GetValue() const { return value_; }

  private:
    T value_;
};

// 1.8.2 可变参数类模板 - Variant 风格（仅示例）
template <typename... Types> class SimpleVariant {
  public:
    template <typename T> SimpleVariant(T value) {
        // 实际实现需要更复杂的存储机制，这里仅示意
        static_assert((std::is_same_v<T, Types> || ...), "Type must be one of the variant types");
    }
};

// ----------------------------------------------------------------------------
// 1.9 参数包索引访问
// ----------------------------------------------------------------------------

// 获取参数包中第 N 个元素
template <std::size_t N, typename T, typename... Args>
decltype(auto) GetNth(T && first, Args &&... rest) {
    if constexpr (N == 0) {
        return std::forward<T>(first);
    } else {
        return GetNth<N - 1>(std::forward<Args>(rest)...);
    }
}

// ----------------------------------------------------------------------------
// 1.10 可变参数 Lambda (C++14+)
// ----------------------------------------------------------------------------

auto CreateVariadicLambda() {
    // 泛型 lambda 可以接受任意数量的参数
    return [](auto &&... args) {
        fmt::print("Lambda 参数个数: {}\n", sizeof...(args));
        ((fmt::print("{} ", std::forward<decltype(args)>(args))), ...);
        fmt::print("\n");
    };
}

// ----------------------------------------------------------------------------
// 1.11 可变参数与初始化列表结合
// ----------------------------------------------------------------------------

template <typename... Args> void PrintWithBraces(Args... args) {
    std::initializer_list<int> list{
        (fmt::print("{} ", args), 0)... // 打印每个参数，返回0
    };
    fmt::print("\n");
}

// ----------------------------------------------------------------------------
// 1.12 递归模板实例化 - 编译期计算
// ----------------------------------------------------------------------------

// 编译期求和
template <int... Nums> struct CompileTimeSum;

template <> struct CompileTimeSum<> {
    static constexpr int value = 0;
};

template <int First, int... Rest> struct CompileTimeSum<First, Rest...> {
    static constexpr int value = First + CompileTimeSum<Rest...>::value;
};

// ----------------------------------------------------------------------------
// 1.13 混合可变参数：类型和非类型参数
// ----------------------------------------------------------------------------

template <typename... Types, int... Values> void MixedVariadic() {
    fmt::print("类型参数个数: {}, 非类型参数个数: {}\n", sizeof...(Types), sizeof...(Values));
}

// 实际使用需要分开声明
template <int... Values> void NonTypeVariadic() {
    fmt::print("非类型参数: ");
    ((fmt::print("{} ", Values)), ...);
    fmt::print("\n");
}

} // namespace cpp_variadic

// ============================================================================
// 第二部分：C 语言可变参数 (stdarg.h)
// ============================================================================

namespace c_variadic {

// ----------------------------------------------------------------------------
// 2.1 基本用法 - va_start, va_arg, va_end
// ----------------------------------------------------------------------------

// 计算整数之和（参数个数作为第一个参数）
int SumIntegers(int count, ...) {
    va_list args; // 定义参数列表
    va_start(args, count); // 初始化参数列表，count是最后一个具名参数

    int sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += va_arg(args, int); // 获取下一个int类型的参数
    }

    va_end(args); // 清理参数列表
    return sum;
}

// ----------------------------------------------------------------------------
// 2.2 处理不同类型的参数 - 使用格式字符串
// ----------------------------------------------------------------------------

// 自定义 printf 风格函数（简化版）
void CustomPrintf(const char * format, ...) {
    va_list args;
    va_start(args, format);

    for (const char * p = format; *p != '\0'; ++p) {
        if (*p != '%') {
            fmt::print("{}", *p);
            continue;
        }

        switch (*++p) { // 跳过 '%'，检查格式符
            case 'd':
                {
                    int i = va_arg(args, int);
                    fmt::print("{}", i);
                    break;
                }
            case 'f':
                {
                    double d = va_arg(args, double); // float 会被提升为 double
                    fmt::print("{:.2f}", d);
                    break;
                }
            case 's':
                {
                    const char * s = va_arg(args, const char *);
                    fmt::print("{}", s);
                    break;
                }
            case 'c':
                {
                    int c = va_arg(args, int); // char 会被提升为 int
                    fmt::print("{}", static_cast<char>(c));
                    break;
                }
            default:
                fmt::print("{}", *p);
                break;
        }
    }

    va_end(args);
}

// ----------------------------------------------------------------------------
// 2.3 可变参数的最大值
// ----------------------------------------------------------------------------

// 求任意多个整数的最大值（第一个参数是个数）
int MaxOfIntegers(int count, ...) {
    if (count <= 0) {
        return 0;
    }

    va_list args;
    va_start(args, count);

    int max_val = va_arg(args, int); // 第一个值作为初始最大值
    for (int i = 1; i < count; ++i) {
        int current = va_arg(args, int);
        if (current > max_val) {
            max_val = current;
        }
    }

    va_end(args);
    return max_val;
}

// ----------------------------------------------------------------------------
// 2.4 va_copy - 复制参数列表
// ----------------------------------------------------------------------------

// 计算平均值：需要遍历两次参数列表
double Average(int count, ...) {
    if (count <= 0) {
        return 0.0;
    }

    va_list args;
    va_start(args, count);

    // 第一次遍历：求和
    double  sum = 0.0;
    va_list args_copy;
    va_copy(args_copy, args); // 复制参数列表

    for (int i = 0; i < count; ++i) {
        sum += va_arg(args, double);
    }

    va_end(args);

    // 如果需要再次遍历，可以使用复制的列表
    // 这里不再使用，直接清理
    va_end(args_copy);

    return sum / count;
}

// ----------------------------------------------------------------------------
// 2.5 字符串拼接
// ----------------------------------------------------------------------------

// 拼接多个字符串（NULL 结尾标记）
std::string ConcatenateStrings(const char * first, ...) {
    std::string result = first;

    va_list args;
    va_start(args, first);

    const char * str;
    while ((str = va_arg(args, const char *)) != nullptr) {
        result += str;
    }

    va_end(args);
    return result;
}

// ----------------------------------------------------------------------------
// 2.6 可变参数与数组
// ----------------------------------------------------------------------------

// 计算多个double数组的元素之和
// 参数：数组个数，每个数组的长度，数组指针1, 数组指针2, ...
double SumArrays(int array_count, int length, ...) {
    va_list args;
    va_start(args, length);

    double sum = 0.0;
    for (int i = 0; i < array_count; ++i) {
        const double * arr = va_arg(args, const double *);
        for (int j = 0; j < length; ++j) {
            sum += arr[j];
        }
    }

    va_end(args);
    return sum;
}

// ----------------------------------------------------------------------------
// 2.7 嵌套可变参数函数调用
// ----------------------------------------------------------------------------

// 辅助函数：使用 va_list 参数
int SumFromVaList(int count, va_list args) {
    int sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += va_arg(args, int);
    }
    return sum;
}

// 包装函数：调用辅助函数
int WrapperSum(int count, ...) {
    va_list args;
    va_start(args, count);
    int result = SumFromVaList(count, args);
    va_end(args);
    return result;
}

// ----------------------------------------------------------------------------
// 2.8 类型不安全的陷阱示例（仅用于教学）
// ----------------------------------------------------------------------------

// 错误示例：期望 int 但传入了 double
int UnsafeSum(int count, ...) {
    va_list args;
    va_start(args, count);

    int sum = 0;
    for (int i = 0; i < count; ++i) {
        // 如果实际传入的是 double，这里会导致未定义行为
        sum += va_arg(args, int);
    }

    va_end(args);
    return sum;
}

} // namespace c_variadic

// ============================================================================
// 主函数 - 演示所有用法
// ============================================================================

int main() {
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    spdlog::info("=== C++ 和 C 语言可变参数示例 ===\n");

    // --------------------------------------------------------------------------
    // C++ 可变参数演示
    // --------------------------------------------------------------------------

    spdlog::info("【第一部分：C++ 可变参数模板】");
    fmt::print("\n");

    // 1.1 递归展开
    fmt::print("1.1 递归展开打印:\n    ");
    cpp_variadic::PrintRecursive(1, 2.5, "hello", 'A', true);

    // 1.2 参数个数
    fmt::print("1.2 ");
    cpp_variadic::PrintCount(10, 20, 30, 40, 50);

    // 1.3 逗号表达式展开
    fmt::print("1.3 逗号表达式展开:\n    ");
    cpp_variadic::PrintCommaExpansion("C++", 17, 3.14, 'X');

    // 1.4 折叠表达式
    fmt::print("1.4 折叠表达式:\n");
    fmt::print("    右折叠求和 (1+2+3+4+5): {}\n", cpp_variadic::SumFoldRight(1, 2, 3, 4, 5));
    fmt::print("    左折叠求和 (1+2+3+4+5): {}\n", cpp_variadic::SumFoldLeft(1, 2, 3, 4, 5));
    fmt::print("    带初始值求和 (1+2+3+0): {}\n", cpp_variadic::SumWithInit(1, 2, 3));
    fmt::print("    折叠打印: ");
    cpp_variadic::PrintFold("fold", "expression", "demo", 2024);
    fmt::print("    AllTrue(true, true, false): {}\n", cpp_variadic::AllTrue(true, true, false));
    fmt::print("    AnyTrue(false, false, true): {}\n", cpp_variadic::AnyTrue(false, false, true));

    // 1.5 参数包展开到容器
    fmt::print("1.5 参数包展开到容器:\n");
    auto vec = cpp_variadic::PackToVector(10, 20, 30, 40, 50);
    fmt::print("    Vector: ");
    for (int v : vec) {
        fmt::print("{} ", v);
    }
    fmt::print("\n");

    fmt::print("    应用函数到每个参数:\n    ");
    cpp_variadic::ApplyToEach([](auto x) { fmt::print("[{}] ", x); }, "apple", "banana", "cherry");
    fmt::print("\n");

    // 1.6 完美转发
    fmt::print("1.6 完美转发:\n    ");
    auto lambda = [](int a, double b, const std::string & c) {
        return fmt::format("a={}, b={:.1f}, c={}", a, b, c);
    };
    std::string result = cpp_variadic::Invoke(lambda, 42, 3.14, std::string("test"));
    fmt::print("    结果: {}\n", result);

    // 1.7 类型萃取
    fmt::print("1.7 类型萃取:\n");
    fmt::print("    AreAllSame<int, int, int>: {}\n",
               cpp_variadic::AreAllSame<int, int, int>::value);
    fmt::print("    AreAllSame<int, double, int>: {}\n",
               cpp_variadic::AreAllSame<int, double, int>::value);
    fmt::print("    SumIntegers(10, 20, 30): {}\n", cpp_variadic::SumIntegers(10, 20, 30));

    // 1.8 可变参数模板类
    fmt::print("1.8 可变参数模板类:\n");
    cpp_variadic::SimpleTuple<int, double, std::string> tuple(42, 3.14, "data");
    fmt::print("    SimpleTuple 第一个值: {}\n", tuple.GetValue());

    // 1.9 参数包索引访问
    fmt::print("1.9 参数包索引访问:\n");
    fmt::print("    第 0 个参数: {}\n", cpp_variadic::GetNth<0>("first", "second", "third"));
    fmt::print("    第 1 个参数: {}\n", cpp_variadic::GetNth<1>("first", "second", "third"));
    fmt::print("    第 2 个参数: {}\n", cpp_variadic::GetNth<2>("first", "second", "third"));

    // 1.10 可变参数 Lambda
    fmt::print("1.10 可变参数 Lambda:\n     ");
    auto varLambda = cpp_variadic::CreateVariadicLambda();
    varLambda(1, 2, 3, "test", 4.5);

    // 1.11 初始化列表结合
    fmt::print("1.11 初始化列表结合:\n     ");
    cpp_variadic::PrintWithBraces(100, 200, 300, 400);

    // 1.12 编译期计算
    fmt::print("1.12 编译期求和:\n");
    fmt::print("     CompileTimeSum<1, 2, 3, 4, 5>::value = {}\n",
               cpp_variadic::CompileTimeSum<1, 2, 3, 4, 5>::value);

    // 1.13 非类型参数
    fmt::print("1.13 非类型参数:\n     ");
    cpp_variadic::NonTypeVariadic<10, 20, 30, 40>();

    fmt::print("\n");

    // --------------------------------------------------------------------------
    // C 语言可变参数演示
    // --------------------------------------------------------------------------

    spdlog::info("【第二部分：C 语言可变参数】");
    fmt::print("\n");

    // 2.1 基本用法
    fmt::print("2.1 基本用法 (SumIntegers):\n");
    int sum1 = c_variadic::SumIntegers(5, 10, 20, 30, 40, 50);
    fmt::print("    Sum of 5 integers: {}\n", sum1);

    // 2.2 格式化输出
    fmt::print("2.2 自定义 printf:\n    ");
    c_variadic::CustomPrintf("整数: %d, 浮点: %f, 字符串: %s, 字符: %c\n", 42, 3.14159, "Hello",
                             'A');

    // 2.3 最大值
    fmt::print("2.3 最大值 (MaxOfIntegers):\n");
    int max_val = c_variadic::MaxOfIntegers(7, 15, 42, 8, 99, 23, 56, 31);
    fmt::print("    Max of 7 integers: {}\n", max_val);

    // 2.4 平均值
    fmt::print("2.4 平均值 (Average):\n");
    double avg = c_variadic::Average(4, 10.0, 20.0, 30.0, 40.0);
    fmt::print("    Average of 4 doubles: {:.2f}\n", avg);

    // 2.5 字符串拼接
    fmt::print("2.5 字符串拼接:\n");
    std::string concatenated = c_variadic::ConcatenateStrings("Hello", " ", "World", "!", nullptr);
    fmt::print("    Result: {}\n", concatenated);

    // 2.6 数组求和
    fmt::print("2.6 多个数组求和:\n");
    double arr1[]    = { 1.0, 2.0, 3.0 };
    double arr2[]    = { 4.0, 5.0, 6.0 };
    double arr3[]    = { 7.0, 8.0, 9.0 };
    double array_sum = c_variadic::SumArrays(3, 3, arr1, arr2, arr3);
    fmt::print("    Sum of 3 arrays: {:.1f}\n", array_sum);

    // 2.7 嵌套调用
    fmt::print("2.7 嵌套可变参数函数:\n");
    int sum2 = c_variadic::WrapperSum(4, 100, 200, 300, 400);
    fmt::print("    WrapperSum result: {}\n", sum2);

    // 2.8 类型安全警告
    fmt::print("2.8 类型安全注意事项:\n");
    fmt::print("    C 语言可变参数不进行类型检查！\n");
    fmt::print("    必须通过格式字符串或参数个数等方式确保类型正确。\n");
    fmt::print("    错误的类型会导致未定义行为。\n");

    fmt::print("\n");
    spdlog::info("=== 演示完成 ===");

    return 0;
}
