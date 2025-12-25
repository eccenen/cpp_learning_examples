#include <iostream>
#include <utility> // 实际上 std::forward 在 <utility> 中定义，但很多编译器在 <iostream> 中间接包含，建议显式包含

// 模板类 MyDelegate：用于封装“对象 + 成员函数指针”，形成可调用对象（Callable）
// - T: 类类型（如 struct A）
// - R: 成员函数的返回类型（如 void）
// - Args...: 成员函数的参数类型包（如 int, 或 int, double）
template <class T, class R, typename... Args> class MyDelegate {
  public:
    // 构造函数：接收一个对象指针 t 和一个指向 T 类成员函数 f 的指针
    // 注意：成员函数指针的语法是 R (T::*f)(Args...)
    MyDelegate(T * t, R (T::*f)(Args...)) : m_t(t), m_f(f) {}

    // 重载函数调用运算符 operator()，使 MyDelegate 对象可以像函数一样被调用
    // 参数使用完美转发（universal reference + std::forward）以保持值类别（左值/右值）
    R operator()(Args &&... args) {
        // 使用指向成员函数的指针语法调用：
        // (对象指针 ->* 成员函数指针)(参数...)
        // std::forward 用于完美转发每个参数，避免不必要的拷贝或保持右值语义
        return (m_t->*m_f)(std::forward<Args>(args)...);
    }

  private:
    T * m_t; // 指向目标对象的指针（如 &a）
    R (T::*m_f)(Args...); // 指向 T 类中某个成员函数的指针（如 &A::Fun）
};

// 辅助函数模板：用于自动推导模板参数，简化 MyDelegate 的创建
// 输入：对象指针 t 和成员函数指针 f
// 返回：一个 MyDelegate 对象，自动推导出 T, R, Args...
template <class T, class R, typename... Args>
MyDelegate<T, R, Args...> CreateDelegate(T * t, R (T::*f)(Args...)) {
    // 直接构造并返回 MyDelegate 实例
    // C++17 起可省略模板参数（类模板参数推导），但此处显式写出以兼容旧标准
    return MyDelegate<T, R, Args...>(t, f);
}

//===========================================================================
// ✅ C++17 关键部分：类模板参数推导指引（Deduction Guide）
//===========================================================================
// 模板类 MyDelegate2：封装“对象指针 + 成员函数指针”，形成可调用对象
template <class T, class R, typename... Args> class MyDelegate2 {
  public:
    // 构造函数：接收对象指针和成员函数指针
    // 注意：成员函数指针类型为 R (T::*)(Args...)
    MyDelegate2(T * obj, R (T::*func)(Args...)) : m_obj(obj), m_func(func) {}

    // 重载 operator()，支持完美转发参数
    R operator()(Args &&... args) {
        // 使用 ->* 调用成员函数，并完美转发参数
        return (m_obj->*m_func)(std::forward<Args>(args)...);
    }

  private:
    T * m_obj; // 指向目标对象的指针
    R (T::*m_func)(Args...); // 指向成员函数的指针
};

// ✅ C++17 关键：为 MyDelegate2 提供推导指引（deduction guide）
// 使得编译器能从构造参数自动推导模板参数 T, R, Args...
template <class T, class R, typename... Args>
MyDelegate2(T *, R (T::*)(Args...)) -> MyDelegate2<T, R, Args...>;

// 注意：
// - 这个推导指引告诉编译器：
//   当调用 MyDelegate2(ptr, mem_fn_ptr) 时，
//   自动实例化为 MyDelegate2<T, R, Args...>
// - 如果构造函数是 public 且参数匹配，C++17 有时可以自动推导，
//   但成员函数指针类型较复杂，显式提供 deduction guide 更可靠、可移植。

// 测试类
struct A {
    void Fun(int i) { std::cout << "Fun(" << i << ")\n"; }

    void Fun1(int i, double j) { std::cout << "Fun1(" << i << ", " << j << ")\n"; }

    // 示例：const 成员函数（如果需要支持，需额外重载或推导指引）
    void FunConst(int i) const { std::cout << "FunConst(" << i << ") const\n"; }
};

// 如果你想支持 const 成员函数，可以添加对应的推导指引和构造函数重载（可选）
// 例如：
/*
template <class T, class R, typename... Args>
class MyDelegate2 {
public:
    // 原有非 const 版本
    MyDelegate2(T* obj, R (T::*func)(Args...)) : m_obj(obj), m_func(func) {}

    // 新增 const 成员函数支持
    MyDelegate2(const T* obj, R (T::*func)(Args...) const)
        : m_obj(const_cast<T*>(obj)), m_func_const(func) {}

    // 注意：需要区分存储 const / non-const 成员函数指针，实现会更复杂
    // 为简化，本例仅支持非 const 成员函数
};
*/

int main() {
    A a;

    // ✅ C++17：直接使用类模板，无需 CreateDelegate！
    // 编译器自动推导：T = A, R = void, Args... = int
    MyDelegate2 d1(&a, &A::Fun);
    d1(42); // 调用 a.Fun(42)

    // 自动推导：T = A, R = void, Args... = int, double
    MyDelegate2 d2(&a, &A::Fun1);
    d2(10, 3.14); // 调用 a.Fun1(10, 3.14)

    // 也可以用 auto（但通常没必要，因为类型已明确）
    auto d3 = MyDelegate2(&a, &A::Fun);
    d3(99);

    //===================================================================
    // 创建委托 d，绑定对象 a 和成员函数 A::Fun
    // CreateDelegate 自动推导出：T=A, R=void, Args...=int
    auto d = CreateDelegate(&a, &A::Fun);

    // 调用委托 d，等价于 a.Fun(1)
    // d 是可调用对象，其 operator() 被调用，并转发参数 1 给 a.Fun
    d(1); // 输出: Function Fun called with 1

    // 创建另一个委托 d1，绑定对象 a 和成员函数 A::Fun1
    // 推导出：T=A, R=void, Args...=int, double
    auto d4 = CreateDelegate(&a, &A::Fun1);

    // 调用 d1(1, 2.5)，等价于 a.Fun1(1, 2.5)
    d4(1, 2.5); // 输出: Function Fun1 called with 1 and 2.5

    return 0;

    return 0;
}
