#include <iostream>
#include <memory>

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
    void initialize() override { std::cout << "Chess initialized.\n"; }

    void makeMove() override {
        std::cout << "Chess move " << moves + 1 << "\n";
        ++moves;
    }

    bool isFinished() override { return moves >= maxMoves; }

    void printWinner() override { std::cout << "Chess winner: Player 1\n"; }

  private:
    int moves;
    int maxMoves;
};

class Soccer : public AbstractGame {
  public:
    Soccer() : minutes(0), maxMinutes(2) {}

  protected:
    void initialize() override { std::cout << "Soccer match initialized.\n"; }

    void makeMove() override {
        std::cout << "Soccer minute " << minutes + 1 << "\n";
        ++minutes;
    }

    bool isFinished() override { return minutes >= maxMinutes; }

    void printWinner() override { std::cout << "Soccer winner: Home team\n"; }

  private:
    int minutes;
    int maxMinutes;
};

int main() {
    std::unique_ptr<AbstractGame> game1 = std::make_unique<Chess>();
    std::cout << "Running Chess:\n";
    game1->run();

    std::cout << "\n";

    std::unique_ptr<AbstractGame> game2 = std::make_unique<Soccer>();
    std::cout << "Running Soccer:\n";
    game2->run();

    return 0;
}
