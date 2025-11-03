// csrc/memory_pool/basic/03_smart_pointer_usage.cpp
#include "../common/memory_pool_common.h"

using namespace memory_pool;

// ============================================================================
// 智能指针使用场景演示
// ============================================================================

class Resource {
  public:
    explicit Resource(std::string name) : name_(std::move(name)) {
        spdlog::info("  → Resource '{}' 创建", name_);
    }

    ~Resource() { spdlog::info("  ← Resource '{}' 销毁", name_); }

    void Use() const { spdlog::info("  ⚙ 使用 Resource '{}'", name_); }

    const std::string & GetName() const { return name_; }

  private:
    std::string name_;
};

// ============================================================================
// 1. unique_ptr：独占所有权
// ============================================================================

void DemoUniquePtr() {
    spdlog::info("\n=== 1. unique_ptr：独占所有权 ===\n");

    // 创建 unique_ptr
    auto ptr1 = std::make_unique<Resource>("unique_res1");
    ptr1->Use();

    // 转移所有权
    auto ptr2 = std::move(ptr1);
    if (!ptr1) {
        spdlog::info("✓ ptr1 已失效");
    }
    ptr2->Use();

    // unique_ptr 数组
    auto arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
    }
    spdlog::info("数组: [{}, {}, {}, {}, {}]", arr[0], arr[1], arr[2], arr[3], arr[4]);

    // 自动释放（离开作用域）
}

// ============================================================================
// 2. shared_ptr：共享所有权
// ============================================================================

void DemoSharedPtr() {
    spdlog::info("\n=== 2. shared_ptr：共享所有权 ===\n");

    std::shared_ptr<Resource> sp1;

    {
        sp1 = std::make_shared<Resource>("shared_res1");
        spdlog::info("引用计数: {}", sp1.use_count());

        auto sp2 = sp1; // 共享所有权
        spdlog::info("引用计数: {}", sp1.use_count());

        {
            auto sp3 = sp1;
            spdlog::info("引用计数: {}", sp1.use_count());
            sp3->Use();
        } // sp3 销毁

        spdlog::info("引用计数: {}", sp1.use_count());
    } // sp2 销毁

    spdlog::info("引用计数: {}", sp1.use_count());
    sp1->Use();
} // sp1 销毁，资源释放

// ============================================================================
// 3. weak_ptr：解决循环引用
// ============================================================================

struct Node {
    std::string           name;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node>   prev; // 使用 weak_ptr 打破循环

    explicit Node(std::string n) : name(std::move(n)) { spdlog::info("  → Node '{}' 创建", name); }

    ~Node() { spdlog::info("  ← Node '{}' 销毁", name); }
};

void DemoWeakPtr() {
    spdlog::info("\n=== 3. weak_ptr：解决循环引用 ===\n");

    auto node1 = std::make_shared<Node>("Node1");
    auto node2 = std::make_shared<Node>("Node2");

    // 建立双向链接
    node1->next = node2;
    node2->prev = node1; // weak_ptr，不增加引用计数

    spdlog::info("node1 引用计数: {}", node1.use_count());
    spdlog::info("node2 引用计数: {}", node2.use_count());

    // 通过 weak_ptr 访问
    if (auto prev = node2->prev.lock()) {
        spdlog::info("✓ 从 node2 访问到 {}", prev->name);
    }

    // 离开作用域，正确释放
}

// ============================================================================
// 4. 自定义删除器
// ============================================================================

void CustomDeleter(Resource * ptr) {
    spdlog::info("  🗑️ 自定义删除器调用");
    delete ptr;
}

void DemoCustomDeleter() {
    spdlog::info("\n=== 4. 自定义删除器 ===\n");

    // unique_ptr with custom deleter
    {
        std::unique_ptr<Resource, decltype(&CustomDeleter)> ptr(new Resource("custom_delete_res"),
                                                                CustomDeleter);
        ptr->Use();
    }

    // shared_ptr with lambda deleter
    {
        auto ptr = std::shared_ptr<Resource>(new Resource("lambda_delete_res"), [](Resource * p) {
            spdlog::info("  🗑️ Lambda 删除器调用");
            delete p;
        });
        ptr->Use();
    }
}

// ============================================================================
// 5. 工厂模式与智能指针
// ============================================================================

class ResourceFactory {
  public:
    static std::unique_ptr<Resource> CreateUnique(const std::string & name) {
        return std::make_unique<Resource>(name);
    }

    static std::shared_ptr<Resource> CreateShared(const std::string & name) {
        return std::make_shared<Resource>(name);
    }
};

void DemoFactoryPattern() {
    spdlog::info("\n=== 5. 工厂模式与智能指针 ===\n");

    auto unique_res = ResourceFactory::CreateUnique("factory_unique");
    unique_res->Use();

    auto shared_res = ResourceFactory::CreateShared("factory_shared");
    shared_res->Use();
}

// ============================================================================
// 6. 性能对比：原始指针 vs 智能指针
// ============================================================================

void DemoPerformanceComparison() {
    spdlog::info("\n=== 6. 性能对比 ===\n");

    constexpr size_t iterations = 1000000;

    // 原始指针
    {
        Timer timer;
        for (size_t i = 0; i < iterations; ++i) {
            int * ptr = new int(42);
            delete ptr;
        }
        spdlog::info("原始指针:  {:.2f} ms", timer.elapsedMs());
    }

    // unique_ptr
    {
        Timer timer;
        for (size_t i = 0; i < iterations; ++i) {
            auto ptr = std::make_unique<int>(42);
        }
        spdlog::info("unique_ptr: {:.2f} ms", timer.elapsedMs());
    }

    // shared_ptr
    {
        Timer timer;
        for (size_t i = 0; i < iterations; ++i) {
            auto ptr = std::make_shared<int>(42);
        }
        spdlog::info("shared_ptr: {:.2f} ms", timer.elapsedMs());
    }
}

// ============================================================================
// 7. 常见陷阱
// ============================================================================

void DemoCommonPitfalls() {
    spdlog::info("\n=== 7. 常见陷阱 ===\n");

    // 陷阱1：从原始指针创建多个 shared_ptr
    spdlog::info("\n陷阱1: 重复包装原始指针");
    {
        Resource * raw = new Resource("trap1");
        // 错误：会导致 double free
        // auto sp1 = std::shared_ptr<Resource>(raw);
        // auto sp2 = std::shared_ptr<Resource>(raw);  // 危险！
        spdlog::warn("⚠ 不要从同一个原始指针创建多个 shared_ptr");
        delete raw;
    }

    // 陷阱2：shared_ptr 管理栈对象
    spdlog::info("\n陷阱2: shared_ptr 管理栈对象");
    {
        // 错误：栈对象不能用 shared_ptr 管理
        // int stack_value = 42;
        // auto sp = std::shared_ptr<int>(&stack_value, [](int*){}); // 需要空删除器
        spdlog::warn("⚠ shared_ptr 应该管理堆对象");
    }

    // 陷阱3：循环引用
    spdlog::info("\n陷阱3: 循环引用（已在 weak_ptr 演示中解决）");
    spdlog::info("✓ 使用 weak_ptr 打破循环");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    spdlog::set_pattern("[%^%l%$] %v");

    spdlog::info("╔════════════════════════════════════════════════════════╗");
    spdlog::info("║              智能指针使用场景演示                  ║");
    spdlog::info("╚════════════════════════════════════════════════════════╝");

    DemoUniquePtr();
    DemoSharedPtr();
    DemoWeakPtr();
    DemoCustomDeleter();
    DemoFactoryPattern();
    DemoPerformanceComparison();
    DemoCommonPitfalls();

    spdlog::info("\n✓ 所有演示完成！");
    spdlog::info("\n💡 最佳实践：");
    spdlog::info("   1. 默认使用 unique_ptr");
    spdlog::info("   2. 需要共享时使用 shared_ptr");
    spdlog::info("   3. 打破循环引用使用 weak_ptr");
    spdlog::info("   4. 使用 make_unique/make_shared");

    return 0;
}
