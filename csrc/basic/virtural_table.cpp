#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <ios>
#include <iostream>

namespace {
using func_t = void (*)();

class Base {
  public:
    Base(){};

    virtual void getX() { std::cout << "Base::getX()" << std::endl; }

    virtual void getY() { std::cout << "Base::getY()" << std::endl; }

    virtual void getZ() { std::cout << "Base::getZ()" << std::endl; }

    virtual ~Base(){};
};

class Derived : public Base {
  public:
    Derived(){};

    void getX() override { std::cout << "Derived::getX()" << std::endl; }

    void getY() override { std::cout << "Derived::getY()" << std::endl; }

    ~Derived(){};
};

func_t getAddr(void * obj, unsigned int offset) {
    std::cout << "=======================" << std::endl;
    //64位系统下，指针占8字节，因此通过(size_t *)强制转换为size_t指针类型
    auto * vptr = (size_t *) *((size_t *) obj);
    std::cout << "vptr address: 0x" << std::hex << (uintptr_t) vptr << std::endl;
    //通过vptr指针访问virtual table，因为虚表中每个元素(虚函数指针)在64位编译器下是8个字节，
    //因此通过*vptr取出前8字节， 后面加上偏移量就是每个函数的地址！

    void * func = (void *) *(vptr + offset);
    std::cout << "func_tc address: 0x" << std::hex << (uintptr_t) func << std::dec << std::endl;
    return (func_t) func;
}

}  // namespace

int main(void) {
    Base *                pt = new Derived();
    std::function<void()> func;
    func = getAddr(pt, 0);
    func();
    func = getAddr(pt, 1);
    func();
    func = getAddr(pt, 2);
    func();

    delete pt;
    return 0;
}
