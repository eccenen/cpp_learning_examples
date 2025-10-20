#include <iostream>
#include <memory>

// Policy-based Template Method
// The game behavior is provided by a Policy type (compile-time composition).

/*
将行为注入为模板参数（Policy），基类模板持有或使用 Policy
类型来实现算法骨架。优点：高灵活性、零开销抽象；适用于策略在编译期已知场景。
*/

template <typename Policy> class GamePolicy {
  public:
    void run() {
        policy.initialize();
        while (!policy.isFinished()) {
            policy.makeMove();
        }
        policy.printWinner();
    }

  private:
    Policy policy;
};

struct ChessPolicy {
    void initialize() { std::cout << "[Policy] Chess initialized.\n"; }

    void makeMove() {
        std::cout << "[Policy] Chess move\n";
        ++moves;
    }

    bool isFinished() { return moves >= maxMoves; }

    void printWinner() { std::cout << "[Policy] Chess winner\n"; }

    int moves    = 0;
    int maxMoves = 2;
};

struct SoccerPolicy {
    void initialize() { std::cout << "[Policy] Soccer initialized.\n"; }

    void makeMove() {
        std::cout << "[Policy] Soccer minute\n";
        ++minutes;
    }

    bool isFinished() { return minutes >= maxMinutes; }

    void printWinner() { std::cout << "[Policy] Soccer winner\n"; }

    int minutes    = 0;
    int maxMinutes = 1;
};

int main() {
    GamePolicy<ChessPolicy> gc;
    std::cout << "Running [Policy] Chess:\n";
    gc.run();

    std::cout << "\n";

    GamePolicy<SoccerPolicy> gs;
    std::cout << "Running [Policy] Soccer:\n";
    gs.run();

    return 0;
}
