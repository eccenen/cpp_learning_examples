#include <iostream>
#include <memory>

// Policy-based Template Method
// The game behavior is provided by a Policy type (compile-time composition).

/*
将行为注入为模板参数（Policy），基类模板持有或使用 Policy
类型来实现算法骨架。优点：高灵活性、零开销抽象；适用于策略在编译期已知场景。
*/

// --- 策略1：所有权管理策略 ---
// 独占所有权策略 (类似 unique_ptr)
struct ExclusiveOwnership {
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
struct ReferenceCounting {
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
struct StrictChecking {
    template <typename T> static T * checkPointer(T * ptr) {
        if (!ptr) {
            throw std::runtime_error("Null pointer dereference!");
        }
        return ptr;
    }
};

// 空指针检查策略（不检查）
struct NoChecking {
    template <typename T> static T * checkPointer(T * ptr) {
        return ptr; // 直接返回，不做检查
    }
};

// --- 宿主类：SmartPointer ---
template <typename T, typename OwnershipPolicy = ExclusiveOwnership,
          typename CheckingPolicy = StrictChecking>
class SmartPointer : private OwnershipPolicy,
                     private CheckingPolicy {
  private:
    T * ptr_;

  public:
    // 构造函数
    explicit SmartPointer(T * p = nullptr) : ptr_(p) { OwnershipPolicy::incrementReference(ptr_); }

    // 析构函数
    ~SmartPointer() { OwnershipPolicy::decrementReference(ptr_); }

    // 拷贝构造（根据所有权策略行为不同）
    SmartPointer(const SmartPointer & other) : ptr_(other.ptr_) {
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

    void print() const { std::cout << "MyClass: " << value << std::endl; }
};

// --- 使用示例 ---
int main() {
    // 1. 类似 unique_ptr 的智能指针：独占所有权 + 严格检查
    SmartPointer<MyClass, ExclusiveOwnership, StrictChecking> ptr1(new MyClass(42));
    ptr1->print(); // 正常工作
    std::cout << "Is unique? " << ptr1.isUnique() << std::endl; // 1 (true)

    // 2. 类似 shared_ptr 的智能指针：引用计数 + 严格检查
    SmartPointer<MyClass, ReferenceCounting, StrictChecking> ptr2(new MyClass(100));
    {
        auto ptr3 = ptr2; // 拷贝，引用计数增加
        ptr3->print();
        std::cout << "Is unique? " << ptr2.isUnique() << std::endl; // 0 (false)
    } // ptr3 析构，引用计数减少

    // 3. 高性能版本：独占所有权 + 无检查 (用于性能关键场景)
    SmartPointer<MyClass, ExclusiveOwnership, NoChecking> ptr4(new MyClass(999));
    // (*ptr4) 不会进行空指针检查

    // 4. 尝试解引用空指针（会抛出异常）
    try {
        SmartPointer<MyClass, ExclusiveOwnership, StrictChecking> null_ptr(nullptr);
        null_ptr->print(); // 这里会抛出异常
    } catch (const std::exception & e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    return 0;
}
