// Copyright 2025 The cpp-qa-lab Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// 本示例文件系统性展示 C++ 中与函数调用相关的多种机制：
// 1. 传统函数指针 (Free Function Pointer)
// 2. std::function （类型擦除的可调用包装器）
// 3. std::bind （参数绑定 / 重排）
// 4. 成员函数指针与 std::invoke
// 5. 递归与 std::function（含尾递归 / 捕获）
// 6. 可调用组合 / 链式变换
// 7. std::function 的开销、局限（与 move-only lambda 限制示例）
//
// 风格说明：遵循 Google C++ Style Guide：
// - 类型名 / 类名 / 函数名 使用 PascalCase
// - 变量名、参数名 使用 lower_snake_case
// - 常量用 kConstant 风格（本示例中未大量使用）

#include "common.h" // 提供 spdlog / fmt

namespace cpp_qa_lab {
namespace basic {

// ============================================================================
// 1. 函数指针示例 - 基础用法
// ============================================================================

// 声明一个函数指针类型（两个 double 参数，返回 double）
using MathOperation = double (*)(double, double);

double Add(double a, double b) {
    return a + b;
}

double Subtract(double a, double b) {
    return a - b;
}

double Multiply(double a, double b) {
    return a * b;
} // 修正为真正乘法

double Divide(double a, double b) {
    if (b == 0.0) {
        spdlog::warn("Division by zero! Return 0.");
        return 0.0;
    }
    return a / b;
}

double ApplyOperation(double a, double b, MathOperation op) {
    return op(a, b);
}

// ============================================================================
// 2. std::function 示例 - 灵活的类型擦除包装器
// ============================================================================

class Calculator {
  public:
    using OperationFunc = std::function<double(double, double)>;

    Calculator() = default;

    explicit Calculator(OperationFunc op) : operation_(std::move(op)) {}

    double Calculate(double a, double b) const {
        if (operation_) {
            return operation_(a, b);
        }
        spdlog::warn("No operation set! Return 0.");
        return 0.0;
    }

    void SetOperation(OperationFunc op) { operation_ = std::move(op); }

  private:
    OperationFunc operation_;
};

double Power(double base, double exponent) {
    double result = 1.0;
    for (int i = 0; i < static_cast<int>(exponent); ++i) {
        result *= base;
    }
    return result;
}

// 工厂：创建一个乘法因子 lambda
auto CreateMultiplier(double factor) {
    return [factor](double value) {
        return value * factor;
    };
}

// ============================================================================
// 3. std::bind 示例 - 参数重排序 / 部分绑定
// ============================================================================

class Processor {
  public:
    void ProcessData(const std::string & data, int priority) {
        spdlog::info("ProcessData => data:'{}' priority:{}", data, priority);
    }

    void ProcessWithCallback(const std::string &                              data,
                             const std::function<void(const std::string &)> & cb) {
        spdlog::info("ProcessWithCallback => '{}'", data);
        if (cb) {
            cb(data);
        }
    }
};

void LogCallback(const std::string & message) {
    spdlog::info("Callback: {}", message);
}

void ErrorCallback(const std::string & message) {
    spdlog::warn("Error callback: {}", message);
}

// ============================================================================
// 4. 成员函数指针 & std::invoke
// ============================================================================

class Accumulator {
  public:
    explicit Accumulator(int start = 0) : value_(start) {}

    void Add(int delta) { value_ += delta; }

    int Get() const { return value_; }

    // const 成员函数指针演示目标
    int ScaleAndGet(int factor) const { return value_ * factor; }

  private:
    int value_;
};

// ============================================================================
// 5. 递归与 std::function（需要自引用时）
// ============================================================================

int FactorialClassic(int n) {
    return (n <= 1) ? 1 : n * FactorialClassic(n - 1);
}

int FactorialWithStdFunction(int n) {
    std::function<int(int)> fact = [&](int x) -> int {
        return (x <= 1) ? 1 : x * fact(x - 1);
    };
    return fact(n);
}

// 尾递归形式（为了演示，不做尾调用优化保证）
int FactorialTailHelper(int n, int acc) {
    return (n <= 1) ? acc : FactorialTailHelper(n - 1, n * acc);
}

int FactorialTail(int n) {
    return FactorialTailHelper(n, 1);
}

// ============================================================================
// 6. 可调用组合 / 链式变换 (string pipeline)
// ============================================================================

class FunctionChain {
  public:
    using TransformFunc = std::function<std::string(const std::string &)>;

    void AddTransform(TransformFunc f) { transforms_.push_back(std::move(f)); }

    std::string Apply(const std::string & input) const {
        std::string result = input;
        for (const auto & t : transforms_) {
            if (t) {
                result = t(result);
            }
        }
        return result;
    }

  private:
    std::vector<TransformFunc> transforms_;
};

std::string ToUpper(const std::string & s) {
    std::string out = s;
    for (char & c : out) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return out;
}

FunctionChain::TransformFunc AddPrefix(const std::string & prefix) {
    return [prefix](const std::string & s) {
        return prefix + s;
    };
}

FunctionChain::TransformFunc AddSuffix(const std::string & suffix) {
    return [suffix](const std::string & s) {
        return s + suffix;
    };
}

// ============================================================================
// 7. std::function 的局限：move-only 捕获示例（演示说明）
// ============================================================================

// 说明：如果 lambda 捕获 std::unique_ptr（按值），该 lambda 将不可拷贝，
// 无法存入 std::function（std::function 需要可复制目标）。
// 这里用注释形式演示：
//   auto ptr = std::make_unique<int>(42);
//   std::function<void()> bad = [p = std::move(ptr)]() { spdlog::info("{}", *p); }; // ❌ 编译失败
// 解决办法：使用 std::move 进一个 shared_ptr，或用自定义模板包装，或直接存 lambda 到 auto /
// 模板参数。

// ============================================================================
// 演示函数
// ============================================================================

void DemonstrateFunctionPointers() {
    spdlog::info("\n=== 函数指针示例 ===");
    double a = 10.0, b = 5.0;
    spdlog::info("a = {}, b = {}", a, b);

    MathOperation ops[]   = { Add, Subtract, Multiply, Divide };
    const char *  names[] = { "Add", "Subtract", "Multiply", "Divide" };
    for (size_t i = 0; i < 4; ++i) {
        spdlog::info("{}: {}", names[i], ApplyOperation(a, b, ops[i]));
    }
}

void DemonstrateStdFunction() {
    spdlog::info("\n=== std::function 示例 ===");
    Calculator calc(Add); // 初始使用 Add
    spdlog::info("Add via std::function: {}", calc.Calculate(3.0, 4.0));

    calc.SetOperation(Power);
    spdlog::info("Power(2, 5): {}", calc.Calculate(2.0, 5.0));

    calc.SetOperation([](double x, double y) { return x * y + y; });
    spdlog::info("Custom lambda: {}", calc.Calculate(3.0, 6.0));

    auto triple = CreateMultiplier(3.0); // 返回一个 lambda (double)->double
    spdlog::info("Triple(7) = {}", triple(7.0));
}

void DemonstrateStdBind() {
    spdlog::info("\n=== std::bind 示例 ===");
    Processor proc;

    // 参数重排：将 (data, priority) -> 绑定成 (priority_first, data_second)
    auto reordered =
        std::bind(&Processor::ProcessData, &proc, std::placeholders::_2, std::placeholders::_1);
    reordered(1, "High Priority"); // 实际调用：data="High Priority", priority=1

    // 部分绑定：固定优先级 = 5
    auto fixed_priority = std::bind(&Processor::ProcessData, &proc, std::placeholders::_1, 5);
    fixed_priority("Normal Data");

    // 绑定回调
    auto success_cb = std::bind(LogCallback, std::placeholders::_1);
    auto error_cb   = std::bind(ErrorCallback, std::placeholders::_1);
    proc.ProcessWithCallback("Task A", success_cb);
    proc.ProcessWithCallback("Task B", error_cb);

    // 绑定临时 lambda
    auto custom_cb = std::bind([](const std::string & m) { spdlog::info("Custom: {}", m); },
                               std::placeholders::_1);
    proc.ProcessWithCallback("Task C", custom_cb);
}

void DemonstrateMemberFunctionPointerAndInvoke() {
    spdlog::info("\n=== 成员函数指针 & std::invoke 示例 ===");
    Accumulator acc(10);
    void (Accumulator::*add_ptr)(int)        = &Accumulator::Add; // 成员函数指针
    int (Accumulator::*scale_ptr)(int) const = &Accumulator::ScaleAndGet; // const 成员函数

    // 调用方式一：obj.*ptr
    (acc.*add_ptr)(5);
    spdlog::info("After Add via pointer: {}", acc.Get());

    // 调用方式二：std::invoke（统一调用方式）
    std::invoke(add_ptr, acc, 7); // 临时加 7
    spdlog::info("After std::invoke Add: {}", acc.Get());

    int scaled = std::invoke(scale_ptr, acc, 3);
    spdlog::info("ScaleAndGet(3): {}", scaled);
}

void DemonstrateRecursiveStdFunction() {
    spdlog::info("\n=== 递归函数示例 ===");
    int n = 6;
    spdlog::info("FactorialClassic({}) = {}", n, FactorialClassic(n));
    spdlog::info("FactorialWithStdFunction({}) = {}", n, FactorialWithStdFunction(n));
    spdlog::info("FactorialTail({}) = {}", n, FactorialTail(n));
}

void DemonstrateFunctionChain() {
    spdlog::info("\n=== 函数链式组合示例 ===");
    FunctionChain chain;
    chain.AddTransform(ToUpper);
    chain.AddTransform(AddPrefix("PREFIX_"));
    chain.AddTransform(AddSuffix("_SUFFIX"));
    chain.AddTransform(
        [](const std::string & s) { return s + "|LEN=" + std::to_string(s.size()); });

    std::string input  = "hello world";
    std::string output = chain.Apply(input);
    spdlog::info("Input: {}", input);
    spdlog::info("Output: {}", output);
}

void DemonstrateMoveOnlyLimitation() {
    spdlog::info("\n=== std::function Move-Only 限制说明 ===");
    spdlog::info("通过注释展示：捕获 unique_ptr 的 lambda 不能放入 std::function，因为不可复制。");
}

// ============================================================================
// 汇总演示入口
// ============================================================================

void RunAllFunctionDemos() {
    DemonstrateFunctionPointers();
    DemonstrateStdFunction();
    DemonstrateStdBind();
    DemonstrateMemberFunctionPointerAndInvoke();
    DemonstrateRecursiveStdFunction();
    DemonstrateFunctionChain();
    DemonstrateMoveOnlyLimitation();
}

} // namespace basic
} // namespace cpp_qa_lab

int main() {
    spdlog::info("C++ 函数指针 / std::function / std::bind / std::invoke 综合示例");
    spdlog::info("================================================================");
    cpp_qa_lab::basic::RunAllFunctionDemos();
    spdlog::info("\n================================================================");
    spdlog::info("所有函数相关示例演示完成！");
    return 0;
}
