/**
 * 初级教程2：智能指针的正确使用
 *
 * 学习目标：
 * 1. 掌握unique_ptr的独占语义
 * 2. 理解shared_ptr的共享所有权
 * 3. 避免weak_ptr的循环引用
 * 4. 了解智能指针的性能开销
 */

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace beginner {

// 示例类
class Resource {
  public:
    Resource(const std::string & name) : name_(name) {
        spdlog::info("  [构造] Resource: {}", name_);
    }

    ~Resource() { spdlog::info("  [析构] Resource: {}", name_); }

    void use() { spdlog::info("  使用 Resource: {}", name_); }

  private:
    std::string name_;
};

// 示例1：unique_ptr - 独占所有权
void unique_ptr_basics() {
    spdlog::info("\n=== 示例1：unique_ptr - 独占所有权 ===");

    // 创建unique_ptr
    {
        auto ptr1 = std::make_unique<Resource>("unique-1");
        ptr1->use();

        // unique_ptr不可拷贝
        // auto ptr2 = ptr1;  // ❌ 编译错误！

        // 但可以移动
        auto ptr2 = std::move(ptr1);
        if (!ptr1) {
            spdlog::info("✓ ptr1已被移动，现在为nullptr");
        }
        ptr2->use();
    } // ptr2离开作用域，自动释放

    spdlog::info("✓ 离开作用域，Resource自动释放");
}

// 示例2：unique_ptr 用于数组
void unique_ptr_array() {
    spdlog::info("\n=== 示例2：unique_ptr 管理数组 ===");

    // C++11-14: unique_ptr<T[]>
    auto arr = std::make_unique<int[]>(10);
    for (int i = 0; i < 10; ++i) {
        arr[i] = i * i;
    }
    spdlog::info("arr[5] = {}", arr[5]);

    // 注意：优先使用std::vector而不是数组
    std::vector<int> vec(10);
    spdlog::info("✓ 提示：优先使用std::vector代替原始数组");
}

// 示例3：shared_ptr - 共享所有权
void shared_ptr_basics() {
    spdlog::info("\n=== 示例3：shared_ptr - 共享所有权 ===");

    std::shared_ptr<Resource> shared1;

    {
        shared1 = std::make_shared<Resource>("shared-1");
        spdlog::info("引用计数: {}", shared1.use_count());

        {
            auto shared2 = shared1; // 拷贝，引用计数+1
            spdlog::info("引用计数: {}", shared1.use_count());

            auto shared3 = shared1;
            spdlog::info("引用计数: {}", shared1.use_count());
        } // shared2和shared3销毁，引用计数-2

        spdlog::info("引用计数: {}", shared1.use_count());
    }

    spdlog::info("引用计数: {}", shared1.use_count());
    shared1.reset(); // 手动释放
    spdlog::info("✓ 所有引用消失，Resource被释放");
}

// 示例4：循环引用问题
class Node {
  public:
    std::string           name;
    std::shared_ptr<Node> next; // 强引用
    std::weak_ptr<Node>   prev; // 弱引用，打破循环

    Node(const std::string & n) : name(n) { std::cout << "  [构造] Node: " << name << "\n"; }

    ~Node() { std::cout << "  [析构] Node: " << name << "\n"; }
};

void circular_reference_problem() {
    spdlog::info("\n=== 示例4：循环引用与weak_ptr ===");

    // 错误示例：双向强引用导致内存泄漏
    spdlog::warn("⚠️ 错误：双向强引用（已注释，会泄漏）");
    // {
    //     auto node1 = std::make_shared<Node>("Node1");
    //     auto node2 = std::make_shared<Node>("Node2");
    //     node1->next = node2;  // Node1 -> Node2
    //     node2->next = node1;  // Node2 -> Node1 (循环!)
    // }  // 内存泄漏！引用计数永远不为0

    // 正确示例：使用weak_ptr打破循环
    spdlog::info("\n✓ 正确：使用weak_ptr打破循环");
    {
        auto node1 = std::make_shared<Node>("Node-A");
        auto node2 = std::make_shared<Node>("Node-B");

        node1->next = node2; // Node-A -> Node-B (强引用)
        node2->prev = node1; // Node-B -> Node-A (弱引用)

        // 使用weak_ptr
        if (auto prev = node2->prev.lock()) {
            spdlog::info("Node-B的prev是: {}", prev->name);
        }
    } // ✓ 正确释放

    spdlog::info("✓ 所有Node正确释放");
}

// 示例5：自定义删除器
void custom_deleter_example() {
    spdlog::info("\n=== 示例5：自定义删除器 ===");

    // 为C风格API添加RAII
    auto file_deleter = [](FILE * fp) {
        if (fp) {
            spdlog::info("  关闭文件");
            fclose(fp);
        }
    };

    // 模拟文件操作（不实际创建文件）
    spdlog::info("使用自定义删除器管理FILE*");

    // unique_ptr with custom deleter
    {
        using FilePtr = std::unique_ptr<FILE, decltype(file_deleter)>;
        // FilePtr file(fopen("test.txt", "w"), file_deleter);
        spdlog::info("✓ unique_ptr可以使用自定义删除器");
    }

    // shared_ptr with custom deleter (更简单)
    {
        // auto file = std::shared_ptr<FILE>(fopen("test.txt", "w"), file_deleter);
        spdlog::info("✓ shared_ptr的删除器更易使用");
    }
}

// 示例6：性能考虑
void performance_considerations() {
    spdlog::info("\n=== 示例6：智能指针的性能考虑 ===");

    spdlog::info("sizeof(int*) = {} bytes", sizeof(int *));
    spdlog::info("sizeof(unique_ptr<int>) = {} bytes", sizeof(std::unique_ptr<int>));
    spdlog::info("sizeof(shared_ptr<int>) = {} bytes", sizeof(std::shared_ptr<int>));

    spdlog::info("\n性能特点：");
    spdlog::info("• unique_ptr: 零开销抽象，与原始指针性能相同");
    spdlog::info("• shared_ptr: 有引用计数开销，大小是原始指针的2倍");
    spdlog::info("• make_shared: 一次分配，比单独new+shared_ptr更高效");

    // 推荐：优先使用make_unique/make_shared
    auto ptr1 = std::make_unique<Resource>("efficient");
    auto ptr2 = std::make_shared<Resource>("efficient-shared");
}

// 示例7：使用场景选择
void usage_scenarios() {
    spdlog::info("\n=== 示例7：智能指针使用场景 ===");

    spdlog::info("\n何时使用unique_ptr：");
    spdlog::info("✓ 独占资源所有权");
    spdlog::info("✓ 工厂函数返回值");
    spdlog::info("✓ pImpl惯用法");
    spdlog::info("✓ 需要移动但不需要拷贝");

    spdlog::info("\n何时使用shared_ptr：");
    spdlog::info("✓ 多个对象共享资源");
    spdlog::info("✓ 不确定谁最后释放资源");
    spdlog::info("✓ 需要在容器中存储并共享");

    spdlog::info("\n何时使用weak_ptr：");
    spdlog::info("✓ 打破shared_ptr循环引用");
    spdlog::info("✓ 观察者模式");
    spdlog::info("✓ 缓存（可能过期的引用）");

    spdlog::info("\n何时仍使用原始指针：");
    spdlog::info("✓ 不拥有所有权的观察指针");
    spdlog::info("✓ 性能关键的热路径");
    spdlog::info("✓ 与C API交互");
}

} // namespace beginner

void run_smart_pointer_examples() {
    spdlog::info("\n╔════════════════════════════════════════════╗");
    spdlog::info("║  初级：智能指针使用指南                    ║");
    spdlog::info("╚════════════════════════════════════════════╝");

    beginner::unique_ptr_basics();
    beginner::unique_ptr_array();
    beginner::shared_ptr_basics();
    beginner::circular_reference_problem();
    beginner::custom_deleter_example();
    beginner::performance_considerations();
    beginner::usage_scenarios();
}

#ifndef DOCTEST_CONFIG_DISABLE
int main() {
    run_smart_pointer_examples();
    return 0;
}
#endif
