#include "common.h"

// Standard library headers previously included via common.h:


// Template Method pattern example
// AbstractClass defines the template method `run` which calls primitive operations
// Concrete implementations override the primitive operations to provide custom behavior.

class AbstractGame {
  public:
    virtual ~AbstractGame() = default;

    // The template method defines the skeleton of an algorithm.
    void run() {
        initialize();
        while (!isFinished()) {
            makeMove();
        }
        printWinner();
    }

  protected:
    virtual void initialize()  = 0;
    virtual void makeMove()    = 0;
    virtual bool isFinished()  = 0;
    virtual void printWinner() = 0;
};

class Chess : public AbstractGame {
  public:
    Chess() : moves(0), maxMoves(3) {}

  protected:
    void initialize() override { fmt::print("Chess initialized.\n"); }

    void makeMove() override {
        fmt::print("Chess move {}\n", moves + 1);
        ++moves;
    }

    bool isFinished() override { return moves >= maxMoves; }

    void printWinner() override { fmt::print("Chess winner: Player 1\n"); }

  private:
    int moves;
    int maxMoves;
};

class Soccer : public AbstractGame {
  public:
    Soccer() : minutes(0), maxMinutes(2) {}

  protected:
    void initialize() override { fmt::print("Soccer match initialized.\n"); }

    void makeMove() override {
        fmt::print("Soccer minute {}\n", minutes + 1);
        ++minutes;
    }

    bool isFinished() override { return minutes >= maxMinutes; }

    void printWinner() override { fmt::print("Soccer winner: Home team\n"); }

  private:
    int minutes;
    int maxMinutes;
};

int main() {
    std::unique_ptr<AbstractGame> game1 = std::make_unique<Chess>();
    fmt::print("Running Chess:\n");
    game1->run();

    fmt::print("\n");

    std::unique_ptr<AbstractGame> game2 = std::make_unique<Soccer>();
    fmt::print("Running Soccer:\n");
    game2->run();

    return 0;
}
