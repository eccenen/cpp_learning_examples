/**
 * 内存池单元测试
 * 使用内联doctest
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <list>

#include "../advanced/pool_allocator.h"
#include "../intermediate/fixed_block_pool.h"
#include "../intermediate/stack_allocator.h"

using namespace memory_pool;

// ============================================================================
// 固定块池测试
// ============================================================================

TEST_CASE("FixedBlockPool - 基本分配和释放") {
    FixedBlockPool pool(sizeof(int), 10);

    SUBCASE("单次分配") {
        int * ptr = static_cast<int *>(pool.allocate());
        REQUIRE(ptr != nullptr);
        *ptr = 42;
        CHECK(*ptr == 42);
        pool.deallocate(ptr);
    }

    SUBCASE("多次分配") {
        std::vector<void *> ptrs;
        for (int i = 0; i < 5; ++i) {
            void * ptr = pool.allocate();
            REQUIRE(ptr != nullptr);
            ptrs.push_back(ptr);
        }

        for (void * ptr : ptrs) {
            pool.deallocate(ptr);
        }
    }

    SUBCASE("池耗尽") {
        std::vector<void *> ptrs;

        // 分配所有
        for (int i = 0; i < 10; ++i) {
            void * ptr = pool.allocate();
            REQUIRE(ptr != nullptr);
            ptrs.push_back(ptr);
        }

        // 第11次应该失败
        void * overflow = pool.allocate();
        CHECK(overflow == nullptr);

        // 清理
        for (void * ptr : ptrs) {
            pool.deallocate(ptr);
        }
    }
}

TEST_CASE("FixedBlockPool - 内存复用") {
    FixedBlockPool pool(sizeof(int), 5);

    int * p1        = static_cast<int *>(pool.allocate());
    *p1             = 100;
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(p1);

    pool.deallocate(p1);

    // 重新分配应该得到相同地址（复用）
    int *     p2    = static_cast<int *>(pool.allocate());
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(p2);

    CHECK(addr1 == addr2);

    pool.deallocate(p2);
}

TEST_CASE("FixedBlockPool - owns检查") {
    FixedBlockPool pool(sizeof(int), 5);

    int * ptr = static_cast<int *>(pool.allocate());
    CHECK(pool.owns(ptr) == true);

    int external = 42;
    CHECK(pool.owns(&external) == false);
    CHECK(pool.owns(nullptr) == false);

    pool.deallocate(ptr);
}

TEST_CASE("FixedBlockPool - 统计信息") {
    FixedBlockPool pool(64, 10);

    CHECK(pool.stats().allocation_count == 0);
    CHECK(pool.stats().current_usage == 0);

    void * p1 = pool.allocate();
    CHECK(pool.stats().allocation_count == 1);
    CHECK(pool.stats().current_usage == 64);

    void * p2 = pool.allocate();
    CHECK(pool.stats().allocation_count == 2);
    CHECK(pool.stats().current_usage == 128);

    pool.deallocate(p1);
    CHECK(pool.stats().deallocation_count == 1);
    CHECK(pool.stats().current_usage == 64);

    pool.deallocate(p2);
    CHECK(pool.stats().current_usage == 0);
}

// ============================================================================
// 栈分配器测试
// ============================================================================

TEST_CASE("StackAllocator - 基本分配") {
    StackAllocator stack(1024);

    SUBCASE("单次分配") {
        void * ptr = stack.allocate(64);
        REQUIRE(ptr != nullptr);
        CHECK(stack.used() == 64);
    }

    SUBCASE("多次分配") {
        void * p1 = stack.allocate(100);
        CHECK(stack.used() == 100);

        void * p2 = stack.allocate(200);
        CHECK(stack.used() == 300);

        void * p3 = stack.allocate(300);
        CHECK(stack.used() == 600);
    }

    SUBCASE("空间不足") {
        void * p1 = stack.allocate(800);
        REQUIRE(p1 != nullptr);

        void * p2 = stack.allocate(300); // 超出容量
        CHECK(p2 == nullptr);
        CHECK(stack.used() == 800);
    }
}

TEST_CASE("StackAllocator - 标记和恢复") {
    StackAllocator stack(1024);

    void * p1 = stack.allocate(100);
    CHECK(stack.used() == 100);

    auto marker = stack.get_marker();

    void * p2 = stack.allocate(200);
    void * p3 = stack.allocate(300);
    CHECK(stack.used() == 600);

    stack.free_to_marker(marker);
    CHECK(stack.used() == 100);
}

TEST_CASE("StackAllocator - 清空") {
    StackAllocator stack(1024);

    stack.allocate(100);
    stack.allocate(200);
    stack.allocate(300);
    CHECK(stack.used() == 600);

    stack.clear();
    CHECK(stack.used() == 0);
    CHECK(stack.available() == 1024);
}

TEST_CASE("StackAllocator - 对齐分配") {
    StackAllocator stack(1024);

    // 分配1字节
    void * p1 = stack.allocate(1);

    // 分配16字节对齐
    void *    p2   = stack.allocate(64, 16);
    uintptr_t addr = reinterpret_cast<uintptr_t>(p2);
    CHECK(addr % 16 == 0);

    // 分配64字节对齐
    void * p3 = stack.allocate(128, 64);
    addr      = reinterpret_cast<uintptr_t>(p3);
    CHECK(addr % 64 == 0);
}

TEST_CASE("StackAllocator - RAII作用域") {
    StackAllocator stack(1024);

    stack.allocate(100);
    CHECK(stack.used() == 100);

    {
        StackAllocatorScope scope(stack);
        stack.allocate(200);
        CHECK(stack.used() == 300);
    } // 自动恢复

    CHECK(stack.used() == 100);
}

// ============================================================================
// STL分配器测试
// ============================================================================

TEST_CASE("PoolAllocator - 基本功能") {
    SUBCASE("与std::vector") {
        std::vector<int, PoolAllocator<int>> vec;

        for (int i = 0; i < 10; ++i) {
            vec.push_back(i);
        }

        CHECK(vec.size() == 10);
        CHECK(vec[5] == 5);
    }

    SUBCASE("与std::list") {
        std::list<int, SimplePoolAllocator<int>> lst;

        for (int i = 0; i < 10; ++i) {
            lst.push_back(i * 10);
        }

        CHECK(lst.size() == 10);
        CHECK(lst.front() == 0);
        CHECK(lst.back() == 90);
    }
}

TEST_CASE("PoolAllocator - Rebind") {
    using IntAlloc    = PoolAllocator<int>;
    using DoubleAlloc = typename std::allocator_traits<IntAlloc>::rebind_alloc<double>;

    // 确保rebind编译通过
    DoubleAlloc alloc;
    double *    ptr = alloc.allocate(1);
    REQUIRE(ptr != nullptr);
    *ptr = 3.14;
    CHECK(*ptr == doctest::Approx(3.14));
    alloc.deallocate(ptr, 1);
}

// ============================================================================
// 辅助函数测试
// ============================================================================

TEST_CASE("辅助函数 - align_up") {
    CHECK(align_up(0, 4) == 0);
    CHECK(align_up(1, 4) == 4);
    CHECK(align_up(3, 4) == 4);
    CHECK(align_up(4, 4) == 4);
    CHECK(align_up(5, 4) == 8);
    CHECK(align_up(7, 8) == 8);
    CHECK(align_up(9, 8) == 16);
}

TEST_CASE("辅助函数 - is_power_of_two") {
    CHECK(is_power_of_two(1) == true);
    CHECK(is_power_of_two(2) == true);
    CHECK(is_power_of_two(4) == true);
    CHECK(is_power_of_two(8) == true);
    CHECK(is_power_of_two(16) == true);

    CHECK(is_power_of_two(0) == false);
    CHECK(is_power_of_two(3) == false);
    CHECK(is_power_of_two(5) == false);
    CHECK(is_power_of_two(6) == false);
    CHECK(is_power_of_two(7) == false);
}

TEST_CASE("PoolStats - 记录统计") {
    PoolStats stats;

    CHECK(stats.allocation_count == 0);
    CHECK(stats.current_usage == 0);
    CHECK(stats.peak_usage == 0);

    stats.record_allocation(100);
    CHECK(stats.allocation_count == 1);
    CHECK(stats.current_usage == 100);
    CHECK(stats.peak_usage == 100);

    stats.record_allocation(200);
    CHECK(stats.allocation_count == 2);
    CHECK(stats.current_usage == 300);
    CHECK(stats.peak_usage == 300);

    stats.record_deallocation(100);
    CHECK(stats.deallocation_count == 1);
    CHECK(stats.current_usage == 200);
    CHECK(stats.peak_usage == 300); // 峰值不变
}

// ============================================================================
// 压力测试
// ============================================================================

TEST_CASE("压力测试 - 大量分配和释放") {
    FixedBlockPool      pool(32, 10000);
    std::vector<void *> ptrs;

    // 分配5000个
    for (int i = 0; i < 5000; ++i) {
        void * ptr = pool.allocate();
        REQUIRE(ptr != nullptr);
        ptrs.push_back(ptr);
    }

    CHECK(pool.stats().allocation_count == 5000);

    // 随机释放一半
    for (int i = 0; i < 2500; ++i) {
        pool.deallocate(ptrs[i * 2]);
    }

    CHECK(pool.stats().deallocation_count == 2500);

    // 再次分配
    for (int i = 0; i < 2500; ++i) {
        void * ptr = pool.allocate();
        REQUIRE(ptr != nullptr);
    }

    // 清理
    for (int i = 1; i < 5000; i += 2) {
        pool.deallocate(ptrs[i]);
    }
    for (int i = 0; i < 2500; ++i) {
        pool.deallocate(ptrs[i * 2]);
    }
}
