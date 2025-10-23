// stack_only.cpp
// Demonstrates how to make a class that can only be created on the stack.
// Techniques:
//  - delete operator new to prevent heap allocation
//  - keep constructor public so stack allocation is allowed

#include "common.h"

class StackOnly {
  public:
    StackOnly(int v) : value(v) { std::cout << "StackOnly constructed: " << value << "\n"; }

    ~StackOnly() { std::cout << "StackOnly destroyed: " << value << "\n"; }

    // Delete heap allocation
    static void * operator new(std::size_t) = delete;
    static void operator delete(void *)     = delete;

    int getValue() const { return value; }

  private:
    int value;
};

int main_stack_only() {
    // Valid: created on stack
    StackOnly s(10);
    std::cout << "value=" << s.getValue() << "\n";

    // Invalid (will not compile if uncommented):
    // auto p = new StackOnly(5); // error: operator new is deleted

    return 0;
}

// Provide a small main to run this example directly when built as an
// executable.
int main() {
    return main_stack_only();
}
