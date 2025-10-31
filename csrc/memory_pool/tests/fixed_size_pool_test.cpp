// csrc/memory_pool/intermediate/04_fixed_size_pool_demo.cpp
#include "../common/memory_pool_common.h"
#include "../intermediate/fixed_size_pool.h"

using namespace memory_pool;

// 测试结构体
struct TestObject {
    int    id;
    double value;
    char   data[16];

    TestObject(int i = 0) : id(i), value(i * 1.5) {
        std::memset(data, 0, sizeof(data));
        spdlog::info("  TestObject[{}] 构造 @ {}", id, static_cast<void *>(this));
    }

    ~TestObject() { spdlog::info("  TestObject[{}] 析构 @ {}", id, static_cast<void *>(this)); }
};

void DemoBasicUsage() {
    spdlog::info("\n=== 1. 基础用法演示 ===\n");

    constexpr size_t pool_size = 10;
    FixedSizePool    pool(sizeof(TestObject), pool_size);

    pool.Visualize();

    // 分配几个对象
    spdlog::info("\n分配 3 个对象：");
    void * p1 = pool.Allocate();
    void * p2 = pool.Allocate();
    void * p3 = pool.Allocate();

    // 使用 placement new 构造对象
    TestObject * obj1 = new (p1) TestObject(1);
    TestObject * obj2 = new (p2) TestObject(2);
    TestObject * obj3 = new (p3) TestObject(3);

    pool.PrintStats();

    // 释放
    spdlog::info("\n释放对象：");
    obj1->~TestObject(); // 显式调用析构函数
    pool.Deallocate(p1);

    obj2->~TestObject();
    pool.Deallocate(p2);

    pool.PrintStats();

    // 清理
    obj3->~TestObject();
    pool.Deallocate(p3);
}

void DemoPoolExhaustion() {
    spdlog::info("\n=== 2. 内存池耗尽演示 ===\n");

    constexpr size_t small_pool_size = 3;
    FixedSizePool    pool(sizeof(int), small_pool_size);

    std::vector<void *> allocated;

    // 尝试分配超过池大小的块
    for (size_t i = 0; i < small_pool_size + 2; ++i) {
        void * ptr = pool.Allocate();
        if (ptr) {
            spdlog::info("✓ 分配成功 #{}: {}", i, ptr);
            allocated.push_back(ptr);
        } else {
            spdlog::warn("❌ 分配失败 #{}: 内存池已耗尽", i);
        }
    }

    pool.PrintStats();

    // 释放
    for (void * ptr : allocated) {
        pool.Deallocate(ptr);
    }
}

void DemoPerformanceComparison() {
    spdlog::info("\n=== 3. 性能对比：内存池 vs new/delete ===\n");

    constexpr size_t iterations  = 10000;
    constexpr size_t object_size = 64;

    // 测试 new/delete
    {
        Timer timer;
        for (size_t i = 0; i < iterations; ++i) {
            void * ptr = ::operator new(object_size);
            ::operator delete(ptr);
        }
        double elapsed = timer.ElapsedMilliseconds();
        spdlog::info("new/delete: {:.3f} ms ({:.0f} ops/ms)", elapsed, iterations / elapsed);
    }

    // 测试内存池
    {
        FixedSizePool pool(object_size, 100); // 复用 100 个块
        Timer         timer;

        for (size_t i = 0; i < iterations; ++i) {
            void * ptr = pool.Allocate();
            pool.Deallocate(ptr);
        }

        double elapsed = timer.ElapsedMilliseconds();
        spdlog::info("内存池:     {:.3f} ms ({:.0f} ops/ms)", elapsed, iterations / elapsed);
    }
}

void DemoMemoryFragmentation() {
    spdlog::info("\n=== 4. 内存碎片演示 ===\n");

    FixedSizePool pool(32, 10);

    // 分配所有块
    std::vector<void *> ptrs;
    for (int i = 0; i < 10; ++i) {
        ptrs.push_back(pool.Allocate());
    }

    spdlog::info("所有块已分配");
    pool.PrintStats();

    // 释放偶数索引的块
    spdlog::info("\n释放偶数索引块：");
    for (size_t i = 0; i < ptrs.size(); i += 2) {
        pool.Deallocate(ptrs[i]);
        ptrs[i] = nullptr;
    }

    pool.PrintStats();
    pool.Visualize();

    // 清理
    for (void * ptr : ptrs) {
        if (ptr) {
            pool.Deallocate(ptr);
        }
    }
}

int main() {
    spdlog::set_pattern("[%^%l%$] %v");

    spdlog::info("╔════════════════════════════════════════════════════════╗");
    spdlog::info("║            固定大小内存池演示                      ║");
    spdlog::info("╚════════════════════════════════════════════════════════╝");

    DemoBasicUsage();
    DemoPoolExhaustion();
    DemoPerformanceComparison();
    DemoMemoryFragmentation();

    spdlog::info("\n✓ 所有演示完成！");

    return 0;
}
