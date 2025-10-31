// csrc/memory_pool/basic/01_raw_pointer_lifecycle.cpp
#include "../common/memory_pool_common.h"
#include "../common/visualizer.h"

using namespace memory_pool;

// ============================================================================
// 1. 基础：new/delete 的工作原理
// ============================================================================

void DemoBasicNewDelete() {
    spdlog::info("\n=== 1. 基础 new/delete 演示 ===\n");

    // 单个对象分配
    int * p1 = new int(42);
    MemoryVisualizer::VisualizeMemoryBlock(p1, sizeof(int), "单个 int");
    spdlog::info("值: {}", *p1);
    delete p1;

    // 数组分配
    int * arr = new int[5]{ 1, 2, 3, 4, 5 };
    MemoryVisualizer::VisualizeMemoryBlock(arr, sizeof(int) * 5, "int 数组");
    spdlog::info("数组元素: [{}, {}, {}, {}, {}]", arr[0], arr[1], arr[2], arr[3], arr[4]);
    delete[] arr; // 注意：必须使用 delete[]

    spdlog::info("✓ 所有内存已正确释放");
}

// ============================================================================
// 2. 对象生命周期管理
// ============================================================================

class Resource {
  public:
    explicit Resource(int id) : id_(id) {
        spdlog::info("  → Resource[{}] 构造 @ {}", id_, static_cast<void *>(this));
    }

    ~Resource() { spdlog::info("  ← Resource[{}] 析构 @ {}", id_, static_cast<void *>(this)); }

    void Use() const { spdlog::info("  ⚙ Resource[{}] 使用中", id_); }

  private:
    int id_;
};

void DemoObjectLifecycle() {
    spdlog::info("\n=== 2. 对象生命周期演示 ===\n");

    spdlog::info("场景 1: 栈对象");
    {
        Resource stack_obj(1);
        stack_obj.Use();
    } // 自动析构

    spdlog::info("\n场景 2: 堆对象");
    Resource * heap_obj = new Resource(2);
    heap_obj->Use();
    delete heap_obj; // 手动析构

    spdlog::info("\n场景 3: 数组对象");
    Resource * obj_array = new Resource[3]{ Resource(3), Resource(4), Resource(5) };
    obj_array[0].Use();
    delete[] obj_array; // 会调用每个对象的析构函数
}

// ============================================================================
// 3. 常见错误：悬空指针
// ============================================================================

void DemoDanglingPointer() {
    spdlog::info("\n=== 3. 悬空指针问题演示 ===\n");

    int * ptr = new int(100);
    spdlog::info("分配内存: {} @ {}", *ptr, static_cast<void *>(ptr));

    delete ptr;
    spdlog::info("内存已释放");

    // 错误：悬空指针访问（未定义行为）
    spdlog::warn("⚠ 悬空指针值（未定义行为）: {}", *ptr); // 危险！

    // 正确做法：置空
    ptr = nullptr;
    if (ptr == nullptr) {
        spdlog::info("✓ 指针已置空，可以安全检查");
    }
}

// ============================================================================
// 4. 常见错误：重复删除
// ============================================================================

void DemoDoubleFree() {
    spdlog::info("\n=== 4. 重复删除问题演示 ===\n");

    int * ptr = new int(200);
    spdlog::info("分配内存: {}", *ptr);

    delete ptr;
    spdlog::info("第一次删除：正常");

    // 错误：重复删除（未定义行为，可能崩溃）
    // delete ptr;  // 取消注释会导致崩溃
    spdlog::warn("⚠ 已注释重复删除代码，否则会崩溃");

    // 正确做法
    ptr = nullptr;
    delete ptr; // 删除 nullptr 是安全的
    spdlog::info("✓ 删除 nullptr 是安全的");
}

// ============================================================================
// 5. 常见错误：数组 delete 不匹配
// ============================================================================

void DemoDeleteMismatch() {
    spdlog::info("\n=== 5. delete/delete[] 不匹配演示 ===\n");

    int * arr = new int[5]{ 1, 2, 3, 4, 5 };
    spdlog::info("分配数组: 5 个元素");

    // 错误：应该使用 delete[]
    // delete arr;  // 未定义行为
    spdlog::warn("⚠ 使用 delete 释放数组是错误的");

    // 正确做法
    delete[] arr;
    spdlog::info("✓ 使用 delete[] 正确释放数组");
}

// ============================================================================
// 6. 内存泄漏检测
// ============================================================================

void DemoMemoryLeakDetection() {
    spdlog::info("\n=== 6. 内存泄漏检测演示 ===\n");

    MemoryStats stats;

    // 模拟分配
    int * p1 = new int(10);
    stats.RecordAllocation(sizeof(int));

    int * p2 = new int(20);
    stats.RecordAllocation(sizeof(int));

    int * p3 = new int(30);
    stats.RecordAllocation(sizeof(int));

    // 只释放部分
    delete p1;
    stats.RecordDeallocation(sizeof(int));

    delete p2;
    stats.RecordDeallocation(sizeof(int));

    // p3 未释放，内存泄漏！
    spdlog::warn("⚠ p3 未释放，造成内存泄漏");

    stats.Print();

    // 清理
    delete p3;
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    spdlog::set_pattern("[%^%l%$] %v");

    spdlog::info("╔════════════════════════════════════════════════════════╗");
    spdlog::info("║          原始指针生命周期管理教学示例              ║");
    spdlog::info("╚════════════════════════════════════════════════════════╝");

    DemoBasicNewDelete();
    DemoObjectLifecycle();
    DemoDanglingPointer();
    DemoDoubleFree();
    DemoDeleteMismatch();
    DemoMemoryLeakDetection();

    spdlog::info("\n✓ 所有演示完成！");
    spdlog::info("\n💡 提示：");
    spdlog::info("   - 使用 valgrind 检测内存泄漏");
    spdlog::info("   - 使用 ASan 检测内存错误");
    spdlog::info("   - 优先使用智能指针而非原始指针");

    return 0;
}
