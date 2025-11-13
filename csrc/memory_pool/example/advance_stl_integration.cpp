/**
 * STL分配器集成示例
 */

#include <spdlog/spdlog.h>

#include <list>
#include <map>
#include <string>
#include <vector>

#include "advance_pool_allocator.h"

using namespace memory_pool;

// 示例1：与std::vector集成
void example_vector_integration() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例1：std::vector与内存池集成        ║");
    spdlog::info("╚════════════════════════════════════════╝");

    // 创建使用自定义分配器的vector
    using IntVector = std::vector<int, PoolAllocator<int>>;

    {
        PoolAllocator<int> alloc(100); // 100个int的池
        IntVector          vec(alloc);

        spdlog::info("\n向vector添加元素:");
        for (int i = 0; i < 10; ++i) {
            vec.push_back(i * 10);
        }

        std::string out;
        for (int val : vec) {
            out += std::to_string(val) + " ";
        }
        spdlog::info("Vector内容: {}", out);
        spdlog::info("Vector大小: {}", vec.size());
        spdlog::info("Vector容量: {}", vec.capacity());
    }

    spdlog::info("\n注意：vector可能需要连续内存，不总是适合固定块池");
}

// 示例2：与std::list集成
void example_list_integration() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例2：std::list与内存池集成          ║");
    spdlog::info("╚════════════════════════════════════════╝");

    using IntList = std::list<int, SimplePoolAllocator<int>>;

    {
        IntList mylist;

        spdlog::info("\n向list添加元素:");
        for (int i = 0; i < 5; ++i) {
            mylist.push_back(i * 100);
        }

        std::string out2;
        for (int val : mylist) {
            out2 += std::to_string(val) + " ";
        }
        spdlog::info("List内容: {}", out2);

        spdlog::info("\n删除一个元素:");
        mylist.pop_front();

        std::string out3;
        for (int val : mylist) {
            out3 += std::to_string(val) + " ";
        }
        spdlog::info("List内容: {}", out3);
    }

    spdlog::info("\n✓ list非常适合内存池，每个节点大小固定");
}

// 示例3：自定义类型
#include <fmt/format.h>

struct Point {
    double x, y, z;

    Point(double x_ = 0, double y_ = 0, double z_ = 0) : x(x_), y(y_), z(z_) {}

    friend std::ostream & operator<<(std::ostream & os, const Point & p) {
        return os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
    }
};

template <> struct fmt::formatter<Point> {
    constexpr auto parse(format_parse_context & ctx) -> format_parse_context::iterator {
        return ctx.begin();
    }

    auto format(const Point & p, format_context & ctx) const -> format_context::iterator {
        return format_to(ctx.out(), "({}, {}, {})", p.x, p.y, p.z);
    }
};

void example_custom_types() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例3：自定义类型与内存池             ║");
    spdlog::info("╚════════════════════════════════════════╝");

    using PointList = std::list<Point, SimplePoolAllocator<Point>>;

    {
        PointList points;

        spdlog::info("\n添加Point对象:");
        points.emplace_back(1.0, 2.0, 3.0);
        points.emplace_back(4.0, 5.0, 6.0);
        points.emplace_back(7.0, 8.0, 9.0);

        spdlog::info("\nPoints内容:");
        for (const auto & p : points) {
            spdlog::info("  {}", p);
        }
    }
}

// 示例4：性能对比
void example_performance_comparison() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例4：性能对比                       ║");
    spdlog::info("╚════════════════════════════════════════╝");

    const size_t NUM_ELEMENTS = 10000;

    // 标准分配器
    {
        Timer          timer;
        std::list<int> stdlist;

        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            stdlist.push_back(i);
        }

        double push_time = timer.elapsed_us();
        timer.reset();

        stdlist.clear();

        double clear_time = timer.elapsed_us();

        spdlog::info("\n标准std::allocator:");
        spdlog::info("  插入 {} 个元素: {} μs", NUM_ELEMENTS, push_time);
        spdlog::info("  清空: {} μs", clear_time);
        spdlog::info("  总计: {} μs", (push_time + clear_time));
    }

    // 池分配器
    {
        Timer                                    timer;
        std::list<int, SimplePoolAllocator<int>> poollist;

        for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
            poollist.push_back(i);
        }

        double push_time = timer.elapsed_us();
        timer.reset();

        poollist.clear();

        double clear_time = timer.elapsed_us();

        spdlog::info("\nSimplePoolAllocator:");
        spdlog::info("  插入 {} 个元素: {} μs", NUM_ELEMENTS, push_time);
        spdlog::info("  清空: {} μs", clear_time);
        spdlog::info("  总计: {} μs", (push_time + clear_time));
    }
}

// 示例5：分配器特性
void example_allocator_traits() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例5：分配器特性                     ║");
    spdlog::info("╚════════════════════════════════════════╝");

    using Alloc  = PoolAllocator<int>;
    using Traits = std::allocator_traits<Alloc>;

    spdlog::info("PoolAllocator<int> 特性:");
    spdlog::info("  value_type: {}", typeid(Traits::value_type).name());
    spdlog::info("  size_type: {}", typeid(Traits::size_type).name());
    spdlog::info("  propagate_on_container_move: {}",
                 Traits::propagate_on_container_move_assignment::value);
    spdlog::info("  is_always_equal: {}", Traits::is_always_equal::value);

    // Rebind测试
    using ReInt    = Traits::rebind_alloc<int>;
    using ReDouble = Traits::rebind_alloc<double>;

    spdlog::info("\nRebind测试:");
    spdlog::info("  rebind<int>: {}", typeid(ReInt).name());
    spdlog::info("  rebind<double>: {}", typeid(ReDouble).name());
}

// 示例6：容器间共享池
void example_shared_pool() {
    spdlog::info("\n╔════════════════════════════════════════╗");
    spdlog::info("║ 示例6：多容器共享一个池               ║");
    spdlog::info("╚════════════════════════════════════════╝");

    FixedBlockPool shared_pool(sizeof(int), 1000);

    {
        PoolAllocator<int> alloc1(&shared_pool);
        PoolAllocator<int> alloc2(&shared_pool);

        std::vector<int, PoolAllocator<int>> vec1(alloc1);
        std::vector<int, PoolAllocator<int>> vec2(alloc2);

        spdlog::info("两个vector共享同一个池");
        spdlog::info("alloc1 == alloc2: {}", (alloc1 == alloc2));

        vec1.push_back(100);
        vec2.push_back(200);

        shared_pool.print_status();
    }
}

int main() {
    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║ 高级：STL分配器集成教学示例           ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";

    example_vector_integration();
    example_list_integration();
    example_custom_types();
    example_performance_comparison();
    example_allocator_traits();
    example_shared_pool();

    std::cout << "\n\n=== 学习要点总结 ===\n";
    std::cout << "1. STL分配器需要实现特定接口（allocate/deallocate等）\n";
    std::cout << "2. rebind机制允许为不同类型分配内存\n";
    std::cout << "3. list等链表容器最适合固定块池\n";
    std::cout << "4. vector需要连续内存，可能不适合固定块池\n";
    std::cout << "5. 使用allocator_traits简化分配器开发\n";
    std::cout << "6. 多容器可以共享同一个内存池\n";
    std::cout << "7. 自定义分配器可显著提升性能\n";

    return 0;
}
