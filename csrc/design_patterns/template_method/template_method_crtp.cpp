#include <iostream>
#include <memory>

// Template Method using CRTP (Curiously Recurring Template Pattern)
// Static (compile-time) polymorphism: no virtual calls, better performance.

/*
静态多态：通过模板参数把派生类型传入基类（Curiously Recurring Template
Pattern），基类通过static_cast调用派生方法实现模板方法。优点：无虚调用开销，编译期多态；缺点：需要在编译期确定类型，不能存储为同一基类指针集合。
*/
template <typename Derived> class GameCRTP {
  public:
    // Template method
    void run() {
        derived().initialize();
        while (!derived().isFinished()) {
            derived().makeMove();
        }
        derived().printWinner();
    }

  private:
    Derived & derived() { return static_cast<Derived &>(*this); }
};

class ChessCRTP : public GameCRTP<ChessCRTP> {
  public:
    void initialize() { std::cout << "[CRTP] Chess initialized.\n"; }

    void makeMove() {
        std::cout << "[CRTP] Chess move\n";
        ++moves;
    }

    bool isFinished() { return moves >= maxMoves; }

    void printWinner() { std::cout << "[CRTP] Chess winner\n"; }

  private:
    int moves    = 0;
    int maxMoves = 2;
};

class SoccerCRTP : public GameCRTP<SoccerCRTP> {
  public:
    void initialize() { std::cout << "[CRTP] Soccer initialized.\n"; }

    void makeMove() {
        std::cout << "[CRTP] Soccer minute\n";
        ++minutes;
    }

    bool isFinished() { return minutes >= maxMinutes; }

    void printWinner() { std::cout << "[CRTP] Soccer winner\n"; }

  private:
    int minutes    = 0;
    int maxMinutes = 1;
};

int main() {
    ChessCRTP c;
    std::cout << "Running [CRTP] Chess:\n";
    c.run();

    std::cout << "\n";

    SoccerCRTP s;
    std::cout << "Running [CRTP] Soccer:\n";
    s.run();

    return 0;
}
