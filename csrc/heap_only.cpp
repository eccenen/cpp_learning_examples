// heap_only.cpp
// Demonstrates how to make a C++ class that can only be created on the heap.
// Approach: make constructors private or protected and provide a public
// factory that returns std::unique_ptr. Also delete operator new? Not needed.
// heap_only.cpp
// Small example program that uses the HeapOnly class defined in the header.
// heap_only.cpp
// Small example program that uses the HeapOnly class defined in the header.

#include "heap_only.h"

int main() {
    auto p = HeapOnly::create(42);
    p->say();

    auto p2 = new HeapOnly2(84);
    p2->say();
    p2->destroy();

    // HeapOnly2 p3(21);  // error: destructor is private
    return 0;
}
