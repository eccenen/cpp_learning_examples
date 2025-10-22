#include <iostream>
#include <memory>

// Template Method (NVI - Non-Virtual Interface) example
// 基类中将关键的函数声明为private virtual函数，严格地隐藏原语，防止派生类直接调用或误用。
// 派生类只能通过基类的非虚接口调用这些原语。派生类可以重写这些私有虚函数以提供具体行为。

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
    void initialize() override { std::cout << "[NVI] Chess initialized.\n"; }

    void makeMove() override {
        std::cout << "[NVI] Chess move " << moves + 1 << "\n";
        ++moves;
    }

    bool isFinished() override { return moves >= maxMoves; }

    void printWinner() override { std::cout << "[NVI] Chess winner: Player 2\n"; }

  private:
    int moves;
    int maxMoves;
};

class SoccerNVI : public GameNVI {
  public:
    SoccerNVI() : minutes(0), maxMinutes(1) {}

  protected:
    void initialize() override { std::cout << "[NVI] Soccer match initialized.\n"; }

    void makeMove() override {
        std::cout << "[NVI] Soccer minute " << minutes + 1 << "\n";
        ++minutes;
    }

    bool isFinished() override { return minutes >= maxMinutes; }

    void printWinner() override { std::cout << "[NVI] Soccer winner: Away team\n"; }

  private:
    int minutes;
    int maxMinutes;
};

int main() {
    std::unique_ptr<GameNVI> g1 = std::make_unique<ChessNVI>();
    std::cout << "Running [NVI] Chess:\n";
    g1->run();

    std::cout << "\n";

    std::unique_ptr<GameNVI> g2 = std::make_unique<SoccerNVI>();
    std::cout << "Running [NVI] Soccer:\n";
    g2->run();

    return 0;
}
