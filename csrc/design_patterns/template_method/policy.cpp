#include "common.h"

/**
 * @file
 * @brief Policy-based 设计的模板方法示例（Policy-based Template Method）。
 *
 * 说明：通过将行为作为模板参数（Policy）注入，基类模板可以在编
 * 译期组合不同的策略，从而实现高性能、零开销的抽象。通常用于当
 * 行为在编译期已知且性能敏感的场景。
 *
 * 优点：
 * - 零开销抽象：没有虚调用开销，编译器可内联并优化代码；
 * - 高灵活性：通过不同的 Policy 组合可以获得多种行为；
 *
 * 缺点：
 * - 无运行时多态：Policy 在编译期绑定，不能在运行时轻易替换（除
 *   非常规手段）；
 * - 可能导致二进制膨胀（每种 Policy 的组合都生成不同的实例化）；
 *
 * 常见陷阱：
 * - 不要将 Policy 的接口假定为某个基类；最好在文档中声明 Policy
 *   必须提供的方法（例如 incrementReference/decrementReference 等）；
 * - 如果需要在运行时替换策略，请考虑将 Policy 与小型运行时接口
 *   结合使用。
 */

// --- 策略1：所有权管理策略 ---
// 独占所有权策略 (类似 unique_ptr)
struct ExclusiveOwnershipPolicy {
    template <typename PtrType> static void incrementReference(PtrType *) {
        // 独占式无引用计数
    }

    template <typename PtrType> static void decrementReference(PtrType * ptr) {
        delete ptr; // 直接删除资源
    }

    template <typename PtrType> static bool isUnique(const PtrType *) {
        return true; // 独占资源总是“唯一”的
    }
};

// 引用计数策略 (类似 shared_ptr)
struct ReferenceCountingPolicy {
    template <typename PtrType> static void incrementReference(PtrType * ptr) {
        if (ptr) {
            ++(ptr->ref_count); // 假设被指向的对象包含 ref_count 成员
        }
    }

    template <typename PtrType> static void decrementReference(PtrType * ptr) {
        if (ptr && --(ptr->ref_count) == 0) {
            delete ptr;
        }
    }

    template <typename PtrType> static bool isUnique(const PtrType * ptr) {
        return ptr && ptr->ref_count == 1;
    }
};

// --- 策略2：检查策略 ---
// 空指针检查策略（严格）
struct StrictCheckingPolicy {
    template <typename T> static T * checkPointer(T * ptr) {
        if (!ptr) {
            throw std::runtime_error("Null pointer dereference!");
        }
        return ptr;
    }
};

// 空指针检查策略（不检查）
struct NoCheckingPolicy {
    template <typename T> static T * checkPointer(T * ptr) {
        return ptr; // 直接返回，不做检查
    }
};

// --- 宿主类：SmartPointer ---
template <typename T, typename OwnershipPolicy = ExclusiveOwnershipPolicy,
          typename CheckingPolicy = StrictCheckingPolicy>
class SmartPointerPolicy : private OwnershipPolicy,
                           private CheckingPolicy {
  private:
    T * ptr_;

  public:
    // 构造函数
    explicit SmartPointerPolicy(T * p = nullptr) : ptr_(p) {
        OwnershipPolicy::incrementReference(ptr_);
    }

    // 析构函数
    ~SmartPointerPolicy() { OwnershipPolicy::decrementReference(ptr_); }

    // 拷贝构造（根据所有权策略行为不同）
    SmartPointerPolicy(const SmartPointerPolicy & other) : ptr_(other.ptr_) {
        OwnershipPolicy::incrementReference(ptr_);
    }

    // 解引用操作
    T & operator*() const { return *CheckingPolicy::checkPointer(ptr_); }

    T * operator->() const { return CheckingPolicy::checkPointer(ptr_); }

    // 其他方法...
    bool isUnique() const { return OwnershipPolicy::isUnique(ptr_); }
};

// --- 测试用的简单类 ---
class MyClass {
  public:
    int            value;
    // 为了支持ReferenceCounting策略，我们假设它有这个成员
    mutable size_t ref_count = 0;

    MyClass(int v) : value(v) {}

    void print() const { fmt::print("MyClass: {}\n", value); }
};

// --- 使用示例 ---
int main() {
    // 1. 类似 unique_ptr 的智能指针：独占所有权 + 严格检查
    SmartPointerPolicy<MyClass, ExclusiveOwnershipPolicy, StrictCheckingPolicy> ptr1(
        new MyClass(42));
    ptr1->print(); // 正常工作
    fmt::print("Is unique? {}\n", ptr1.isUnique()); // 1 (true)

    // 2. 类似 shared_ptr 的智能指针：引用计数 + 严格检查
    SmartPointerPolicy<MyClass, ReferenceCountingPolicy, StrictCheckingPolicy> ptr2(
        new MyClass(100));
    {
        auto ptr3 = ptr2; // 拷贝，引用计数增加
        ptr3->print();
        fmt::print("Is unique? {}\n", ptr2.isUnique()); // 0 (false)
    } // ptr3 析构，引用计数减少

    // 3. 高性能版本：独占所有权 + 无检查 (用于性能关键场景)
    SmartPointerPolicy<MyClass, ExclusiveOwnershipPolicy, NoCheckingPolicy> ptr4(new MyClass(999));
    // (*ptr4) 不会进行空指针检查

    // 4. 尝试解引用空指针（会抛出异常）
    try {
        SmartPointerPolicy<MyClass, ExclusiveOwnershipPolicy, StrictCheckingPolicy> null_ptr(
            nullptr);
        null_ptr->print(); // 这里会抛出异常
    } catch (const std::exception & e) {
        fmt::print("Caught exception: {}\n", e.what());
    }

    return 0;
}
