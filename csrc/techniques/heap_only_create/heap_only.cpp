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
