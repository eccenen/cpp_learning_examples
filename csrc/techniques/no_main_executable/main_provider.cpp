// main_provider.cpp
// 提供 main 函数入口，类似于 gtest 的 main 实现
// 用户代码只需定义 RunUserTests() 函数，链接此文件即可生成可执行文件

#include "common.h"

// 声明用户需要实现的函数（在其他源文件中定义）
extern int RunUserTests();

int main(int argc, char * argv[]) {
    fmt::print("=== Custom Test Framework ===\n");
    fmt::print("Starting test execution...\n\n");

    // 调用用户定义的测试逻辑
    int result = RunUserTests();

    fmt::print("\n=== Test Execution Completed ===\n");
    fmt::print("Exit code: {}\n", result);

    return result;
}
