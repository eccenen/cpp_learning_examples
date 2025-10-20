#include <functional>
#include <iostream>
#include <memory>

// Template Method via Type Erasure
// We erase the concrete game type and store a set of callables for steps.
// This gives runtime polymorphism without inheritance requirements on the concrete types.

/*
通过将各个步骤封装为
std::function（或类似接口）来擦除具体类型，使不同无共同基类的类型可以在运行时统一调用。优点：运行时多态、无需继承；缺点：一些额外开销（std::function、内存、间接调用），需注意对象状态的共享（我已修复示例中
lambda 捕获的问题，改为捕获 shared_ptr）。
*/
class GameAny {
  public:
    template <typename T> GameAny(T && obj) {
        // Store a shared pointer to a single instance so that each lambda
        // operates on the same object and state changes are visible to all
        // operations (avoid capturing independent copies).
        using U     = std::decay_t<T>;
        auto sp     = std::make_shared<U>(std::forward<T>(obj));
        initialize_ = [sp]() mutable {
            sp->initialize();
        };
        makeMove_ = [sp]() mutable {
            sp->makeMove();
        };
        isFinished_ = [sp]() mutable {
            return sp->isFinished();
        };
        printWinner_ = [sp]() mutable {
            sp->printWinner();
        };
    }

    void run() {
        initialize_();
        while (!isFinished_()) {
            makeMove_();
        }
        printWinner_();
    }

  private:
    std::function<void()> initialize_;
    std::function<void()> makeMove_;
    std::function<bool()> isFinished_;
    std::function<void()> printWinner_;
};

// Two different concrete types without common base
struct ChessLike {
    void initialize() { std::cout << "[TypeErasure] Chess-like init\n"; }

    void makeMove() {
        std::cout << "[TypeErasure] Chess-like move\n";
        ++m;
    }

    bool isFinished() { return m >= 2; }

    void printWinner() { std::cout << "[TypeErasure] Chess-like winner\n"; }

    int m = 0;
};

struct SoccerLike {
    void initialize() { std::cout << "[TypeErasure] Soccer-like init\n"; }

    void makeMove() {
        std::cout << "[TypeErasure] Soccer-like minute\n";
        ++t;
    }

    bool isFinished() { return t >= 1; }

    void printWinner() { std::cout << "[TypeErasure] Soccer-like winner\n"; }

    int t = 0;
};

int main() {
    GameAny g1{ ChessLike{} };
    std::cout << "Running [TypeErasure] ChessLike:\n";
    g1.run();

    std::cout << "\n";

    GameAny g2{ SoccerLike{} };
    std::cout << "Running [TypeErasure] SoccerLike:\n";
    g2.run();

    return 0;
}
