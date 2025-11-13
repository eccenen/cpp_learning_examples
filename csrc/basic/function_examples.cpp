// Copyright 2025 The cpp-qa-lab Authors. All rights reserved.
// C++ 函数与可调用对象综合示例
// 展示：函数指针、std::function、lambda、成员函数指针、std::invoke、递归等

#include "common.h"

// ============================================================================
// 1. 函数指针 - 传统 C 风格函数指针
// ============================================================================

// 基础数学运算函数
double Add(double a, double b) {
    return a + b;
}

double Subtract(double a, double b) {
    return a - b;
}

double Multiply(double a, double b) {
    return a * b;
}

double Divide(double a, double b) {
    return std::abs(b) < 1e-12 ? 0.0 : a / b;
}

// 函数指针类型定义
using BinaryOp = double (*)(double, double);

// 通用二元操作函数，接受函数指针作为参数
double ApplyBinaryOp(double a, double b, BinaryOp op) {
    return op(a, b);
}

void DemoFunctionPointers() {
    spdlog::info("\n=== 1. 函数指针示例 ===");

    BinaryOp     operations[] = { Add, Subtract, Multiply, Divide };
    const char * names[]      = { "加法", "减法", "乘法", "除法" };

    constexpr double x = 10.0, y = 3.0;

    // 遍历函数指针数组
    for (size_t i = 0; i < 4; ++i) {
        double result = operations[i](x, y);
        spdlog::info("{}: {:.1f} 和 {:.1f} = {:.2f}", names[i], x, y, result);
    }

    // 函数指针作为回调参数
    double result = ApplyBinaryOp(15.0, 4.0, Multiply);
    spdlog::info("回调函数结果: 15.0 * 4.0 = {:.1f}", result);
}

// ============================================================================
// 2. std::function 和 Lambda 表达式
// ============================================================================

class Calculator {
  public:
    using Operation = std::function<double(double, double)>;

    void SetOperation(Operation op) { op_ = std::move(op); }

    std::optional<double> Compute(double a, double b) const {
        return op_ ? std::optional<double>(op_(a, b)) : std::nullopt;
    }

  private:
    Operation op_;
};

// 函数组合模板
template <typename F, typename G> auto Compose(F f, G g) {
    return [f = std::move(f), g = std::move(g)](auto &&... args) {
        return f(g(std::forward<decltype(args)>(args)...));
    };
}

void DemoStdFunction() {
    spdlog::info("\n=== 2. std::function 和 Lambda 示例 ===");

    Calculator calc;

    // 使用普通函数
    calc.SetOperation(Add);
    if (auto result = calc.Compute(5.0, 3.0)) {
        spdlog::info("Add(5, 3) = {}", *result);
    }

    // 使用 Lambda 表达式
    calc.SetOperation([](double a, double b) { return std::pow(a, b); });
    if (auto result = calc.Compute(2.0, 8.0)) {
        spdlog::info("Power(2, 8) = {}", *result);
    }

    // 函数组合
    auto double_it = [](double x) {
        return x * 2;
    };
    auto add_ten = [](double x) {
        return x + 10;
    };
    auto composed = Compose(add_ten, double_it);
    spdlog::info("函数组合: (5 * 2) + 10 = {}", composed(5.0));

    // Lambda 捕获示例
    double factor     = 2.5;
    auto   multiplier = [factor](double x) {
        return x * factor;
    };
    spdlog::info("Lambda 捕获: 7.0 * {} = {}", factor, multiplier(7.0));
}

// ============================================================================
// 3. 成员函数指针和 std::invoke
// ============================================================================

class Counter {
  public:
    explicit Counter(int initial = 0) : value_(initial) {}

    void Increment(int delta) { value_ += delta; }

    void Decrement(int delta) { value_ -= delta; }

    int Get() const { return value_; }

    int Multiply(int factor) const { return value_ * factor; }
  private:
    int value_;
};

// 通用调用包装器
template <typename Callable, typename... Args>
auto InvokeAndLog(Callable && callable, Args &&... args) {
    using Result = std::invoke_result_t<Callable, Args...>;

    if constexpr (std::is_void_v<Result>) {
        std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
        spdlog::info("调用了无返回值的可调用对象");
    } else {
        auto result = std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
        spdlog::info("调用结果: {}", result);
        return result;
    }
}

void DemoMemberFunctionPointers() {
    spdlog::info("\n=== 3. 成员函数指针和 std::invoke 示例 ===");

    Counter counter(10);

    // 传统成员函数指针语法
    void (Counter::*inc_ptr)(int) = &Counter::Increment;
    (counter.*inc_ptr)(5);
    spdlog::info("增量后: {}", counter.Get());

    // 使用 std::invoke
    std::invoke(&Counter::Decrement, counter, 3);
    spdlog::info("减量后: {}", counter.Get());

    // 带返回值的调用
    int result = std::invoke(&Counter::Multiply, counter, 4);
    spdlog::info("乘以 4: {}", result);

    // 使用通用调用包装器
    InvokeAndLog(&Counter::Increment, counter, 8);
    InvokeAndLog(&Counter::Get, counter);
}

// ============================================================================
// 4. Lambda 捕获和状态管理
// ============================================================================

class StatefulProcessor {
  public:
    // 创建有状态的累加器
    auto CreateAccumulator(int initial) {
        return [sum = initial](int value) mutable {
            sum += value;
            return sum;
        };
    }

    // 创建只移动的处理器
    auto CreateUniqueHandler(std::unique_ptr<int> ptr) {
        return [p = std::move(ptr)]() {
            return p ? *p : -1;
        };
    }
};

void DemoLambdaCaptures() {
    spdlog::info("\n=== 4. Lambda 捕获和状态管理示例 ===");

    StatefulProcessor processor;

    // 可变 Lambda 维护状态
    auto accumulator = processor.CreateAccumulator(0);
    spdlog::info("累加器: {} -> {} -> {}", accumulator(5), accumulator(10), accumulator(3));

    // 只移动 Lambda
    auto handler = processor.CreateUniqueHandler(std::make_unique<int>(42));
    spdlog::info("唯一处理器: {}", handler());

    // Lambda 只能移动，不能复制
    auto moved_handler = std::move(handler);
    spdlog::info("移动后的处理器: {}", moved_handler());
}

// ============================================================================
// 5. 管道模式和函数变换
// ============================================================================

template <typename T> class Pipeline {
  public:
    using Transform = std::function<T(const T &)>;

    Pipeline & Add(Transform transform) {
        transforms_.push_back(std::move(transform));
        return *this;
    }

    T Execute(const T & input) const {
        return std::accumulate(
            transforms_.begin(), transforms_.end(), input,
            [](const T & value, const Transform & transform) { return transform(value); });
    }

  private:
    std::vector<Transform> transforms_;
};

// 字符串变换工具
std::string ToUpperCase(const std::string & str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

auto AddPrefix(std::string prefix) {
    return [p = std::move(prefix)](const std::string & str) {
        return p + str;
    };
}

auto AddSuffix(std::string suffix) {
    return [s = std::move(suffix)](const std::string & str) {
        return str + s;
    };
}

void DemoPipeline() {
    spdlog::info("\n=== 5. 管道模式示例 ===");

    Pipeline<std::string> pipeline;
    pipeline.Add(ToUpperCase)
        .Add(AddPrefix("["))
        .Add(AddSuffix("]"))
        .Add([](const std::string & s) { return s + " (长度: " + std::to_string(s.size()) + ")"; });

    std::string input  = "hello world";
    std::string output = pipeline.Execute(input);

    spdlog::info("输入: '{}'", input);
    spdlog::info("输出: '{}'", output);
}

// ============================================================================
// 6. 递归和 Y 组合子
// ============================================================================

// 传统递归函数
int Factorial(int n) {
    return (n <= 1) ? 1 : n * Factorial(n - 1);
}

// 使用 std::function 的递归 Lambda
auto MakeRecursiveFibonacci() {
    std::function<int(int)> fib = [&fib](int n) -> int {
        if (n <= 1) {
            return n;
        }
        return fib(n - 1) + fib(n - 2);
    };
    return fib;
}

// Y 组合子实现通用递归
template <typename F> struct YCombinator {
    F func;

    template <typename... Args> auto operator()(Args &&... args) const {
        return func(*this, std::forward<Args>(args)...);
    }
};

template <typename F> YCombinator<std::decay_t<F>> MakeYCombinator(F && func) {
    return { std::forward<F>(func) };
}

void DemoRecursion() {
    spdlog::info("\n=== 6. 递归示例 ===");

    constexpr int n = 7;

    // 传统递归
    spdlog::info("阶乘({}) = {}", n, Factorial(n));

    // 递归 Lambda
    auto fib = MakeRecursiveFibonacci();
    spdlog::info("斐波那契({}) = {}", n, fib(n));

    // Y 组合子
    auto factorial_y =
        MakeYCombinator([](auto self, int n) -> int { return (n <= 1) ? 1 : n * self(n - 1); });
    spdlog::info("Y组合子阶乘({}) = {}", n, factorial_y(n));

    // 另一个 Y 组合子示例
    auto fibonacci_y = MakeYCombinator([](auto self, int n) -> int {
        if (n <= 1) {
            return n;
        }
        return self(n - 1) + self(n - 2);
    });
    spdlog::info("Y组合子斐波那契({}) = {}", n, fibonacci_y(n));
}

// ============================================================================
// 主演示函数
// ============================================================================

void RunAllDemos() {
    spdlog::info("╔════════════════════════════════════════════════════════╗");
    spdlog::info("║          Modern C++ 函数与可调用对象示例              ║");
    spdlog::info("╚════════════════════════════════════════════════════════╝");

    DemoFunctionPointers();
    DemoStdFunction();
    DemoMemberFunctionPointers();
    DemoLambdaCaptures();
    DemoPipeline();
    DemoRecursion();
}

int main() {
    spdlog::set_pattern("[%^%l%$] %v");
    RunAllDemos();
    spdlog::info("\n✓ 所有演示已完成!");
    return 0;
}
