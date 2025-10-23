// main.cpp
// 用户的测试/业务逻辑代码
// 注意：此文件中不包含 main 函数！
// 但通过链接 main_provider.cpp，可以生成可执行文件

#include "common.h"

// 示例：简单的测试用例结构
struct TestCase {
    std::string           name;
    std::function<bool()> test_fn;
};

// 全局测试用例列表
static std::vector<TestCase> g_test_cases;

// 注册测试用例的辅助函数
void RegisterTest(const std::string & name, std::function<bool()> test_fn) {
    g_test_cases.push_back(TestCase{ name, std::move(test_fn) });
}

// 自动注册测试用例的辅助类（类似 gtest 的 TEST 宏）
struct TestRegistrar {
    TestRegistrar(const std::string & name, std::function<bool()> test_fn) {
        RegisterTest(name, std::move(test_fn));
    }
};

// 定义几个测试用例
namespace {

TestRegistrar test1("BasicMath", []() -> bool {
    fmt::print("  Running: BasicMath\n");
    int  result = 2 + 2;
    bool passed = (result == 4);
    fmt::print("    2 + 2 = {}, Expected: 4, Result: {}\n", result, passed ? "PASS" : "FAIL");
    return passed;
});

TestRegistrar test2("StringOps", []() -> bool {
    fmt::print("  Running: StringOps\n");
    std::string str = "Hello";
    str += " World";
    bool passed = (str == "Hello World");
    fmt::print("    String concatenation: '{}', Result: {}\n", str, passed ? "PASS" : "FAIL");
    return passed;
});

TestRegistrar test3("VectorTest", []() -> bool {
    fmt::print("  Running: VectorTest\n");
    std::vector<int> vec    = { 1, 2, 3, 4, 5 };
    bool             passed = (vec.size() == 5 && vec[2] == 3);
    fmt::print("    Vector size: {}, vec[2]: {}, Result: {}\n", vec.size(), vec[2],
               passed ? "PASS" : "FAIL");
    return passed;
});

} // namespace

// 实现 main_provider.cpp 中声明的函数
// 这是 main_provider 会调用的入口函数
int RunUserTests() {
    fmt::print("Discovered {} test case(s)\n\n", g_test_cases.size());

    int passed_count = 0;
    int failed_count = 0;

    for (const auto & test : g_test_cases) {
        fmt::print("[TEST] {}\n", test.name);
        try {
            bool result = test.test_fn();
            if (result) {
                ++passed_count;
            } else {
                ++failed_count;
            }
        } catch (const std::exception & e) {
            fmt::print("    EXCEPTION: {}\n", e.what());
            ++failed_count;
        }
        fmt::print("\n");
    }

    fmt::print("Summary: {} passed, {} failed\n", passed_count, failed_count);
    return (failed_count == 0) ? 0 : 1;
}
