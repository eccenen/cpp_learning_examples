// csrc/memory_pool/basic/02_memory_leak_patterns.cpp
#include "../common/memory_pool_common.h"

using namespace memory_pool;

// ============================================================================
// 常见内存泄漏模式
// ============================================================================

// 模式1：忘记释放
void Pattern1_ForgetToDelete() {
    spdlog::info("\n=== 模式1: 忘记释放内存 ===");
    int * leak = new int(42);
    spdlog::warn("⚠ 分配了内存但忘记 delete");
    // 内存泄漏！
}

// 模式2：异常安全问题
void Pattern2_ExceptionUnsafe() {
    spdlog::info("\n=== 模式2: 异常导致泄漏 ===");
    int * ptr = new int(100);
    try {
        throw std::runtime_error("异常发生");
        delete ptr; // 永远不会执行
    } catch (...) {
        spdlog::warn("⚠ 异常导致 delete 未执行");
    }
}

// 模式3：循环引用（shared_ptr）
struct Node {
    std::shared_ptr<Node> next;

    ~Node() { spdlog::info("Node 析构"); }
};

void Pattern3_CircularReference() {
    spdlog::info("\n=== 模式3: 循环引用 ===");
    auto node1  = std::make_shared<Node>();
    auto node2  = std::make_shared<Node>();
    node1->next = node2;
    node2->next = node1; // 循环引用！
    spdlog::warn("⚠ shared_ptr 循环引用导致泄漏");
}

// 模式4：容器中的裸指针
void Pattern4_RawPointerInContainer() {
    spdlog::info("\n=== 模式4: 容器中的裸指针 ===");
    std::vector<int *> vec;
    vec.push_back(new int(1));
    vec.push_back(new int(2));
    vec.push_back(new int(3));
    // vector 被销毁，但指向的对象未释放
    spdlog::warn("⚠ vector 销毁但内存未释放");
}

// 正确的模式：使用 unique_ptr
void CorrectPattern_UniquePtr() {
    spdlog::info("\n=== ✓ 正确: 使用 unique_ptr ===");
    std::vector<std::unique_ptr<int>> vec;
    vec.push_back(std::make_unique<int>(1));
    vec.push_back(std::make_unique<int>(2));
    vec.push_back(std::make_unique<int>(3));
    spdlog::info("✓ vector 销毁时自动释放内存");
}

int main() {
    spdlog::set_pattern("[%^%l%$] %v");

    spdlog::info("╔════════════════════════════════════════╗");
    spdlog::info("║     常见内存泄漏模式演示           ║");
    spdlog::info("╚════════════════════════════════════════╝");

    Pattern1_ForgetToDelete();
    Pattern2_ExceptionUnsafe();
    Pattern3_CircularReference();
    Pattern4_RawPointerInContainer();
    CorrectPattern_UniquePtr();

    spdlog::info("\n💡 使用 valgrind 运行此程序查看泄漏详情:");
    spdlog::info("   valgrind --leak-check=full ./basic_memory_leak_patterns");

    return 0;
}
