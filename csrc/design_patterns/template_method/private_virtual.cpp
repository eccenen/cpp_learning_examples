#include <fmt/core.h>

#include <memory>

/**
 * @file
 * @brief private virtual 风格的模板方法示例。
 *
 * 说明：在 C++ 中，虚函数可以声明为 private。派生类仍然可以覆写
 * 这些私有虚函数，但不能直接调用它们（因为对派生类不可见）。基
 * 类可以通过受保护的非虚转发方法（hooks）来调用这些私有虚函
 * 数，从而实现更严格的封装。
 *
 * 优点：
 * - 更严格的封装：子类不能直接调用或滥用原语操作，只能通过基类
 *   指定的 hooks 来调用；
 * - 基类对何时以及如何调用这些步骤拥有完全控制权；
 *
 * 缺点与注意事项：
 * - 可读性相对下降（私有虚函数被覆写但是不可见）；
 * - 可能使测试变得复杂（私有虚函数无法直接访问）；
 *
 * 适用场景：当你希望子类只能实现某些步骤但不能直接调用这些步
 * 骤或绕过基类逻辑时。
 */

class GamePrivateVirtual {
  public:
    virtual ~GamePrivateVirtual() = default;

    // Template method: public non-virtual
    void run() {
        // call protected non-virtual hooks that forward to private virtuals
        initHook();
        while (!finishedHook()) {
            moveHook();
        }
        winnerHook();
    }

  protected:
    // Protected non-virtual wrappers forward to private virtuals. This lets
    // the base class control when/how derived overrides are called while
    // keeping the actual virtual functions private.
    void initHook() { doInitialize(); }

    void moveHook() { doMakeMove(); }

    bool finishedHook() { return doIsFinished(); }

    void winnerHook() { doPrintWinner(); }

  private:
    // Private virtual primitive operations. Derived classes can override them
    // but cannot call them directly (they are invoked via the protected hooks).
    virtual void doInitialize()  = 0;
    virtual void doMakeMove()    = 0;
    virtual bool doIsFinished()  = 0;
    virtual void doPrintWinner() = 0;
};

class ChessPV : public GamePrivateVirtual {
  private:
    void doInitialize() override { fmt::print("[PV] Chess initialized.\n"); }

    void doMakeMove() override {
        fmt::print("[PV] Chess move\n");
        ++moves;
    }

    bool doIsFinished() override { return moves >= maxMoves; }

    void doPrintWinner() override { fmt::print("[PV] Chess winner\n"); }

    int moves    = 0;
    int maxMoves = 2;
};

class SoccerPV : public GamePrivateVirtual {
  private:
    void doInitialize() override { fmt::print("[PV] Soccer initialized.\n"); }

    void doMakeMove() override {
        fmt::print("[PV] Soccer minute\n");
        ++minutes;
    }

    bool doIsFinished() override { return minutes >= maxMinutes; }

    void doPrintWinner() override { fmt::print("[PV] Soccer winner\n"); }

    int minutes    = 0;
    int maxMinutes = 1;
};

int main() {
    std::unique_ptr<GamePrivateVirtual> g1 = std::make_unique<ChessPV>();
    fmt::print("Running [PV] Chess:\n");
    g1->run();

    fmt::print("\n");

    std::unique_ptr<GamePrivateVirtual> g2 = std::make_unique<SoccerPV>();
    fmt::print("Running [PV] Soccer:\n");
    g2->run();

    return 0;
}
