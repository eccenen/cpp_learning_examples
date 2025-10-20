#include <iostream>
#include <memory>

// Template Method with private virtual primitives example
// In C++, virtual functions can be private. Derived classes can still override
// them, but they cannot call them directly. This pattern can be used to hide
// primitive operations from the public/protected interface, preventing misuse.

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
    void doInitialize() override { std::cout << "[PV] Chess initialized.\n"; }

    void doMakeMove() override {
        std::cout << "[PV] Chess move\n";
        ++moves;
    }

    bool doIsFinished() override { return moves >= maxMoves; }

    void doPrintWinner() override { std::cout << "[PV] Chess winner\n"; }

    int moves    = 0;
    int maxMoves = 2;
};

class SoccerPV : public GamePrivateVirtual {
  private:
    void doInitialize() override { std::cout << "[PV] Soccer initialized.\n"; }

    void doMakeMove() override {
        std::cout << "[PV] Soccer minute\n";
        ++minutes;
    }

    bool doIsFinished() override { return minutes >= maxMinutes; }

    void doPrintWinner() override { std::cout << "[PV] Soccer winner\n"; }

    int minutes    = 0;
    int maxMinutes = 1;
};

int main() {
    std::unique_ptr<GamePrivateVirtual> g1 = std::make_unique<ChessPV>();
    std::cout << "Running [PV] Chess:\n";
    g1->run();

    std::cout << "\n";

    std::unique_ptr<GamePrivateVirtual> g2 = std::make_unique<SoccerPV>();
    std::cout << "Running [PV] Soccer:\n";
    g2->run();

    return 0;
}
