#include <iostream>
#include <memory>

// Mix of static (compile-time) and dynamic (runtime) polymorphism.
// We use a static template for the main algorithm but accept a runtime strategy
// for a particular primitive step.

/*
在性能敏感的部分使用静态策略（Policy
模板），而在可插拔或运行时选择的部分使用动态策略（接口/多态）。示例展示如何将高频调用做成静态
inline，而把可替换的“move”步骤做成 runtime plugin。
*/
struct IDynamicStep {
    virtual ~IDynamicStep() = default;
    virtual void makeMove() = 0;
};

// FastGame is parameterized by a static Policy for most behavior but calls a
// runtime-provided IDynamicStep for the move step.
template <typename Policy> class FastGame {
  public:
    FastGame(std::shared_ptr<IDynamicStep> dyn) : dynStep(std::move(dyn)) {}

    void run() {
        Policy::initialize();
        while (!Policy::isFinished()) {
            // the performance-sensitive parts can be inlined from Policy,
            // while the dynamic step is a plugin.
            dynStep->makeMove();
        }
        Policy::printWinner();
    }

  private:
    std::shared_ptr<IDynamicStep> dynStep;
};

struct ChessPolicyStatic {
    static void initialize() { std::cout << "[Mixed] Chess initialized.\n"; }

    static bool isFinished() { return moves >= maxMoves; }

    static void printWinner() { std::cout << "[Mixed] Chess winner\n"; }

    static inline int moves    = 0;
    static inline int maxMoves = 2;
};

// A runtime plugin that updates the static policy counters
class ChessMovePlugin : public IDynamicStep {
  public:
    void makeMove() override {
        std::cout << "[Mixed] Chess move (plugin)\n";
        ++ChessPolicyStatic::moves;
    }
};

int main() {
    auto                        plugin = std::make_shared<ChessMovePlugin>();
    FastGame<ChessPolicyStatic> game(plugin);
    std::cout << "Running [Mixed] Chess with static policy + dynamic move plugin:\n";
    game.run();

    return 0;
}
