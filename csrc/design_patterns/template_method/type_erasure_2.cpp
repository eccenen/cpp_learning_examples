#include <fmt/core.h>

#include <any>
#include <functional>
#include <memory>
#include <vector>

namespace { // 类型擦除的模板方法接口

class TypeErasedAlgorithm {
  private:
    struct VTable {
        void (*step1)(std::any &);
        void (*step2)(std::any &);
        void (*step3)(std::any &);
        std::any (*clone)(const std::any &);
        void (*destroy)(std::any &);
    };

    std::any       data_;
    const VTable * vtable_;

  public:
    template <typename T>
    TypeErasedAlgorithm(T && obj) : data_(std::forward<T>(obj)), vtable_(&getVTable<T>()) {}

    // 禁止拷贝（简化版本）
    TypeErasedAlgorithm(const TypeErasedAlgorithm &)             = delete;
    TypeErasedAlgorithm & operator=(const TypeErasedAlgorithm &) = delete;

    // 允许移动
    TypeErasedAlgorithm(TypeErasedAlgorithm && other) noexcept :
        data_(std::move(other.data_)),
        vtable_(other.vtable_) {
        other.vtable_ = nullptr;
    }

    TypeErasedAlgorithm & operator=(TypeErasedAlgorithm && other) noexcept {
        if (this != &other) {
            if (vtable_ && vtable_->destroy) {
                vtable_->destroy(data_);
            }
            data_         = std::move(other.data_);
            vtable_       = other.vtable_;
            other.vtable_ = nullptr;
        }
        return *this;
    }

    ~TypeErasedAlgorithm() {
        if (vtable_ && vtable_->destroy) {
            vtable_->destroy(data_);
        }
    }

    // 模板方法 - 定义算法骨架
    void execute() {
        if (vtable_) {
            fmt::print("Starting algorithm execution...\n");
            vtable_->step1(data_);
            vtable_->step2(data_);
            vtable_->step3(data_);
            fmt::print("Algorithm execution completed!\n");
        }
    }

  private:
    template <typename T> static const VTable & getVTable() {
        static const VTable vt = VTable{ .step1 =
                                             [](std::any & data) {
                                                 T & obj = std::any_cast<T &>(data);
                                                 obj.step1();
                                             },
                                         .step2 =
                                             [](std::any & data) {
                                                 T & obj = std::any_cast<T &>(data);
                                                 obj.step2();
                                             },
                                         .step3 =
                                             [](std::any & data) {
                                                 T & obj = std::any_cast<T &>(data);
                                                 obj.step3();
                                             },
                                         .clone = [](const std::any & data) -> std::any {
                                             const T & obj = std::any_cast<const T &>(data);
                                             return std::any(obj);
                                         },
                                         .destroy =
                                             [](std::any & data) {
                                                 // std::any handles destruction; no-op here
                                             } };
        return vt;
    }
};

} // namespace

namespace { // 改进版本：使用 std::function 增强灵活性

class FlexibleTypeErasedAlgorithm {
  private:
    using StepFunction = std::function<void()>;

    struct AlgorithmData {
        StepFunction step1;
        StepFunction step2;
        StepFunction step3;
        std::string  name;
    };

    AlgorithmData data_;

  public:
    // 通用构造函数
    template <typename T>
    FlexibleTypeErasedAlgorithm(T && obj, std::string name = "") :
        data_{ [obj = std::forward<T>(obj)]() mutable { obj.step1(); },
               [obj = std::forward<T>(obj)]() mutable { obj.step2(); },
               [obj = std::forward<T>(obj)]() mutable { obj.step3(); }, std::move(name) } {}

    // 自定义函数构造函数
    FlexibleTypeErasedAlgorithm(StepFunction s1, StepFunction s2, StepFunction s3,
                                std::string name = "") :
        data_{ std::move(s1), std::move(s2), std::move(s3), std::move(name) } {}

    void execute() {
        if (!data_.name.empty()) {
            fmt::print("Executing: {}\n", data_.name);
        }
        fmt::print("=== Algorithm Start ===\n");
        data_.step1();
        data_.step2();
        data_.step3();
        fmt::print("=== Algorithm End ===\n\n");
    }
};

} // namespace

// 具体算法 A
class ConcreteAlgorithmA {
  public:
    void step1() { fmt::print("ConcreteAlgorithmA: Step 1 - Processing data in way A\n"); }

    void step2() { fmt::print("ConcreteAlgorithmA: Step 2 - Analyzing results in way A\n"); }

    void step3() { fmt::print("ConcreteAlgorithmA: Step 3 - Finalizing in way A\n"); }
};

// 具体算法 B
class ConcreteAlgorithmB {
  private:
    std::string name_;

  public:
    ConcreteAlgorithmB(std::string name = "AlgorithmB") : name_(std::move(name)) {}

    void step1() { fmt::print("ConcreteAlgorithmB[{}]: Step 1 - Fast processing\n", name_); }

    void step2() { fmt::print("ConcreteAlgorithmB[{}]: Step 2 - Quick analysis\n", name_); }

    void step3() { fmt::print("ConcreteAlgorithmB[{}]: Step 3 - Rapid finalization\n", name_); }
};

// 另一个具有不同接口的算法
class LegacyAlgorithm {
  public:
    void process() { fmt::print("LegacyAlgorithm: Custom process\n"); }

    void analyze() { fmt::print("LegacyAlgorithm: Custom analyze\n"); }

    void finish() { fmt::print("LegacyAlgorithm: Custom finish\n"); }
};

// 适配器，让 LegacyAlgorithm 符合我们的接口
class LegacyAlgorithmAdapter {
  private:
    LegacyAlgorithm legacy_;

  public:
    void step1() { legacy_.process(); }

    void step2() { legacy_.analyze(); }

    void step3() { legacy_.finish(); }
};

int main() {
    fmt::print("=== Type Erased Template Method Pattern Demo ===\n\n");

    // 使用基础版本
    fmt::print("1. Basic Version:\n");
    {
        TypeErasedAlgorithm algo1(ConcreteAlgorithmA{});
        algo1.execute();

        TypeErasedAlgorithm algo2(ConcreteAlgorithmB{ "FastProcessor" });
        algo2.execute();

        TypeErasedAlgorithm algo3(LegacyAlgorithmAdapter{});
        algo3.execute();
    }

    fmt::print("\n2. Flexible Version:\n");
    {
        // 使用具体对象
        FlexibleTypeErasedAlgorithm flex1(ConcreteAlgorithmA{}, "Algorithm A");
        flex1.execute();

        // 使用自定义函数
        FlexibleTypeErasedAlgorithm flex2(
            []() { fmt::print("Custom: Step 1\n"); }, []() { fmt::print("Custom: Step 2\n"); },
            []() { fmt::print("Custom: Step 3\n"); }, "Custom Algorithm");
        flex2.execute();

        // 使用 lambda 捕获上下文
        int                         counter = 0;
        FlexibleTypeErasedAlgorithm flex3(
            [&counter]() { fmt::print("Step 1 - Counter: {}\n", ++counter); },
            [&counter]() { fmt::print("Step 2 - Counter: {}\n", ++counter); },
            [&counter]() { fmt::print("Step 3 - Counter: {}\n", ++counter); },
            "Stateful Algorithm");
        flex3.execute();
    }

    fmt::print("\n3. Container of Algorithms:\n");
    {
        std::vector<FlexibleTypeErasedAlgorithm> algorithms;

        algorithms.emplace_back(ConcreteAlgorithmA{}, "Algorithm A");
        algorithms.emplace_back(ConcreteAlgorithmB{ "Processor1" }, "Algorithm B1");
        algorithms.emplace_back(ConcreteAlgorithmB{ "Processor2" }, "Algorithm B2");
        algorithms.emplace_back([]() { fmt::print("Lambda: Step 1\n"); },
                                []() { fmt::print("Lambda: Step 2\n"); },
                                []() { fmt::print("Lambda: Step 3\n"); }, "Lambda Algorithm");

        for (auto & algo : algorithms) {
            algo.execute();
        }
    }

    return 0;
}
