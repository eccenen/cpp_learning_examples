#include "common.h"

// Standard library headers previously included via common.h:


/**
 * @file
 * @brief NVI（Non-Virtual Interface）风格的模板方法示例。
 *
 * 说明：NVI 是一种 C++ 设计技巧，将模板方法设为非虚函数，而将
 * 可变的原语操作声明为受保护的 virtual 方法。这样可以保证算法
 * 骨架不可被子类覆盖，保持行为一致性与封装。
 *
 * 优点：
 * - 模板方法不可被覆写，保证了执行顺序；
 * - 子类可通过覆盖受保护的 virtual 原语实现行为定制；
 *
 * 注意：不要把模板方法声明为 virtual，否则会丧失 NVI 提供的保护。
 */

class GameNVI {
  public:
    virtual ~GameNVI() = default;

    // Non-virtual template method: fixed algorithm skeleton
    void run() {
        initialize();
        while (!isFinished()) {
            makeMove();
        }
        printWinner();
    }

  protected:
    // Primitive operations are virtual and meant to be overridden
    virtual void initialize()  = 0;
    virtual void makeMove()    = 0;
    virtual bool isFinished()  = 0;
    virtual void printWinner() = 0;
};

class ChessNVI : public GameNVI {
  public:
    ChessNVI() : moves(0), maxMoves(2) {}

  protected:
    void initialize() override { fmt::print("[NVI] Chess initialized.\n"); }

    void makeMove() override {
        fmt::print("[NVI] Chess move {}\n", moves + 1);
        ++moves;
    }

    bool isFinished() override { return moves >= maxMoves; }

    void printWinner() override { fmt::print("[NVI] Chess winner: Player 2\n"); }

  private:
    int moves;
    int maxMoves;
};

class SoccerNVI : public GameNVI {
  public:
    SoccerNVI() : minutes(0), maxMinutes(1) {}

  protected:
    void initialize() override { fmt::print("[NVI] Soccer match initialized.\n"); }

    void makeMove() override {
        fmt::print("[NVI] Soccer minute {}\n", minutes + 1);
        ++minutes;
    }

    bool isFinished() override { return minutes >= maxMinutes; }

    void printWinner() override { fmt::print("[NVI] Soccer winner: Away team\n"); }

  private:
    int minutes;
    int maxMinutes;
};

int main() {
    std::unique_ptr<GameNVI> g1 = std::make_unique<ChessNVI>();
    fmt::print("Running [NVI] Chess:\n");
    g1->run();

    fmt::print("\n");

    std::unique_ptr<GameNVI> g2 = std::make_unique<SoccerNVI>();
    fmt::print("Running [NVI] Soccer:\n");
    g2->run();

    return 0;
}
