/**
 * 初级教程1：原始指针的生命周期管理
 *
 * 学习目标：
 * 1. 理解new/delete的配对使用
 * 2. 认识内存泄漏的常见模式
 * 3. 掌握RAII原则
 */

#include <spdlog/spdlog.h>

#include "../common/visualizer.h"
#include "common.h"

using namespace memory_pool;

namespace base {

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

// 示例2：常见的内存泄漏模式
void memory_leak_patterns() {
    spdlog::info("\n=== 示例2：常见的内存泄漏模式 ===\n");

    // 模式1：忘记delete
    {
        int * leak1 = new int(100);
        spdlog::warn("⚠️ 泄漏模式1：忘记delete，分配了但未释放");
        // 缺少 delete leak1;
    }

    // 模式2：异常导致的泄漏
    {
        int * leak2 = new int(200);
        try {
            // 如果这里抛出异常，delete将不会执行
            // throw std::runtime_error("Something went wrong");
            delete leak2; // 这行在异常时不会执行
        } catch (...) {
            spdlog::warn("⚠️ 泄漏模式2：异常导致delete未执行");
        }
    }

    // 模式3：重新赋值导致泄漏
    {
        int * leak3 = new int(300);
        leak3       = new int(400); // 原来的300内存泄漏！
        spdlog::warn("⚠️ 泄漏模式3：重新赋值前未释放原内存");
        delete leak3; // 只释放了400，300已经泄漏
    }

    // 模式4：容器中的原始指针
    {
        int * ptrs[3];
        for (int i = 0; i < 3; ++i) {
            ptrs[i] = new int(i);
        }
        spdlog::warn("⚠️ 泄漏模式4：容器中的指针需要逐个释放");
        // 如果忘记循环delete，全部泄漏
        for (int i = 0; i < 3; ++i) {
            delete ptrs[i]; // 必须记得释放每一个
        }
    }
}

// 示例3：悬空指针问题
void dangling_pointer_problems() {
    spdlog::info("\n=== 示例3：悬空指针问题 ===\n");

    // 问题1：使用已释放的内存
    {
        int * p = new int(42);
        delete p;
        // *p = 100;  // ⚠️ 危险！使用已释放的内存（未定义行为）
        spdlog::warn("⚠️ 悬空指针：delete后不应再使用");
        p = nullptr; // 好习惯：释放后置为nullptr
    }

    // 问题2：多次delete
    {
        int * p = new int(42);
        delete p;
        // delete p;  // ⚠️ 危险！双重释放会导致崩溃
        spdlog::warn("⚠️ 双重释放：同一指针不能delete两次");
        p = nullptr; // 设为nullptr后，delete nullptr是安全的
        delete p; // ✓ 安全，delete nullptr什么都不做
    }

    // 问题3：浅拷贝导致的双重释放
    struct Holder {
        int * data;

        Holder() : data(new int(42)) {}

        ~Holder() { delete data; }

        // 缺少拷贝构造函数和赋值运算符！
    };

    {
        // Holder h1;
        // Holder h2 = h1;  // ⚠️ 浅拷贝！两个对象的data指向同一内存
        // } // 析构时双重释放！

        spdlog::warn("⚠️ 浅拷贝问题：需要实现深拷贝或禁用拷贝");
    }
}

// 示例4：RAII原则 - 自动资源管理
class RAIIBuffer {
  private:
    char * buffer_;
    size_t size_;

  public:
    // 构造时分配
    RAIIBuffer(size_t size) : size_(size) {
        buffer_ = new char[size];
        spdlog::info("分配buffer: {} bytes", size);
    }

    // 析构时释放
    ~RAIIBuffer() {
        delete[] buffer_;
        spdlog::info("释放buffer: {} bytes", size_);
    }

    // 删除拷贝（或实现深拷贝）
    RAIIBuffer(const RAIIBuffer &)             = delete;
    RAIIBuffer & operator=(const RAIIBuffer &) = delete;

    // 允许移动
    RAIIBuffer(RAIIBuffer && other) noexcept : buffer_(other.buffer_), size_(other.size_) {
        other.buffer_ = nullptr;
        other.size_   = 0;
    }

    char * data() { return buffer_; }

    size_t size() const { return size_; }
};

void raii_principle() {
    spdlog::info("\n=== 示例4：RAII原则 - 资源自动管理 ===\n");

    {
        RAIIBuffer buf(1024);
        std::strcpy(buf.data(), "RAII ensures cleanup");
        spdlog::info("使用buffer: {}", buf.data());

        // 即使这里抛出异常，析构函数也会被调用
        // throw std::runtime_error("test");
    } // ✓ 自动调用析构函数，释放内存

    spdlog::info("✓ RAII保证了资源的自动清理");
}

// 示例5：对齐和填充
void alignment_and_padding() {
    spdlog::info("\n=== 示例5：内存对齐和填充 ===\n");

    struct Unaligned {
        char c; // 1 byte
        int  i; // 4 bytes
        char c2; // 1 byte
    };

    struct Aligned {
        int  i; // 4 bytes
        char c; // 1 byte
        char c2; // 1 byte
        // 2 bytes padding
    };

    spdlog::info("Unaligned结构体大小: {} bytes", sizeof(Unaligned));
    spdlog::info("Aligned结构体大小: {} bytes", sizeof(Aligned));
    spdlog::info("int对齐要求: {} bytes", alignof(int));
    spdlog::info("double对齐要求: {} bytes", alignof(double));

    // 手动对齐分配
    void * raw = operator new(1024);
    spdlog::info("原始分配地址: {}", raw);

    size_t space   = 1024;
    void * aligned = std::align(16, 64, raw, space);
    spdlog::info("16字节对齐后地址: {}", aligned);
    spdlog::info("剩余空间: {} bytes", space);

    operator delete(raw);
}

} // namespace base

void run_beginner_examples() {
    spdlog::info("\n╔════════════════════════════════════════════╗\n");
    spdlog::info("║  初级：基础内存管理                        ║\n");
    spdlog::info("╚════════════════════════════════════════════╝\n");

    beginner::correct_raw_pointer_usage();
    beginner::memory_leak_patterns();
    beginner::dangling_pointer_problems();
    beginner::raii_principle();
    beginner::alignment_and_padding();
}

#ifndef DOCTEST_CONFIG_DISABLE
// 主函数
int main() {
    run_beginner_examples();
    return 0;
}
#endif
