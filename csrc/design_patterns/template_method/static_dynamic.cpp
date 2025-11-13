#include "common.h"

/**
 * @file
 * @brief 静态多态 + 动态多态混合示例（Static + Dynamic polymorphism）。
 *
 * 说明：将性能敏感的部分用静态策略（Policy 模板）实现，以便编译
 * 器内联和优化；而将需要在运行时替换的插件部分通过运行时接口（如
 * abstract class）实现，这样兼顾性能和可扩展性。
 *
 * 优点：
 * - 在保证关键路径性能的同时，保留了运行时可替换的能力；
 * - 适用于需要将高频次操作内联但又需要插件化扩展的场景。
 *
 * 缺点：
 * - 设计复杂度增加：需要在静态策略与动态插件之间定义清晰的边界；
 * - 仍需承受部分运行时开销（动态插件的虚函数调用）。
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
    static void initialize() { fmt::print("[Mixed] Chess initialized.\n"); }

    static bool isFinished() { return moves >= maxMoves; }

    static void printWinner() { fmt::print("[Mixed] Chess winner\n"); }

    static inline int moves    = 0;
    static inline int maxMoves = 2;
};

// A runtime plugin that updates the static policy counters
class ChessMovePlugin : public IDynamicStep {
  public:
    void makeMove() override {
        fmt::print("[Mixed] Chess move (plugin)\n");
        ++ChessPolicyStatic::moves;
    }
};

int main() {
    auto                        plugin = std::make_shared<ChessMovePlugin>();
    FastGame<ChessPolicyStatic> game(plugin);
    fmt::print("Running [Mixed] Chess with static policy + dynamic move plugin:\n");
    game.run();

    return 0;
}
