#ifndef CPP_QA_LAB_HEAP_ONLY_H
#define CPP_QA_LAB_HEAP_ONLY_H

// 详细说明见 https://www.yuque.com/linyun-lj2sn/ul5f4n/sm4ger2dovekluo7#RgJwr

#include "common.h"

// HeapOnly: can only be created via the static create() factory which
// returns a std::unique_ptr. Constructor is private and copy is deleted.
class HeapOnly {
  private:
    HeapOnly(int v) : value(v) { std::cout << "HeapOnly constructed: " << value << "\n"; }

  public:
    HeapOnly(const HeapOnly &)             = delete;
    HeapOnly & operator=(const HeapOnly &) = delete;

    ~HeapOnly() { std::cout << "HeapOnly destroyed: " << value << "\n"; }

    static std::unique_ptr<HeapOnly> create(int v) {
        return std::unique_ptr<HeapOnly>(new HeapOnly(v));
        // return std::make_unique<HeapOnly>(v); //error: constructor is private
    }

    void say() const { std::cout << "Hello from HeapOnly: " << value << "\n"; }

    // helper for tests
    int getValue() const { return value; }

  private:
    int value;
};

class HeapOnly2 {
  public:
    HeapOnly2(int v) : value(v) { std::cout << "HeapOnly2 constructed: " << value << "\n"; }

    HeapOnly2(const HeapOnly2 &)             = delete;
    HeapOnly2 & operator=(const HeapOnly2 &) = delete;
    HeapOnly2(HeapOnly2 &&)                  = delete;
    HeapOnly2 & operator=(HeapOnly2 &&)      = delete;

    void say() const { std::cout << "Hello from HeapOnly2: " << value << "\n"; }

    void destroy() {
        std::cout << "HeapOnly2 destroyed: " << value << "\n";
        delete this;
    }

  private:
    ~HeapOnly2() = default; // private destructor
  private:
    int value{};
};

#endif // CPP_QA_LAB_HEAP_ONLY_H
