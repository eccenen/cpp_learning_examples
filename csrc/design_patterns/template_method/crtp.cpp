#include "common.h"

/**
 * @file
 * @brief 基于 CRTP 的模板方法示例与说明。
 *
 * CRTP（Curiously Recurring Template Pattern）/ 静态多态 详细说明。
 *
 * 优点：
 * - 零开销抽象：调用在编译期解析，无虚函数分派开销。编译器可
 *   对调用进行内联和优化；
 * - 适合对性能敏感的代码路径，避免运行时调度成本；
 * - 编译期检查：编译器可以在编译阶段验证派生类型是否实现了所
 *   需的接口方法；
 * - 适用于 mixin 和 policy 风格的组合，在编译期组合行为。
 *
 * 缺点：
 * - 无运行时多态：无法通过基类指针在容器中存储不同派生类型并
 *   期望运行时分派；CRTP 仅提供编译期绑定；
 * - 代码膨胀：对不同 Derived 的每次模板实例化会生成单独函数，
 *   可能增加二进制体积；
 * - 错误信息可能冗长且不直观，尤其是当派生类型未实现预期方法时。
 *
 * 常见陷阱与建议：
 * - 不要将 CRTP 作为虚基类的直接替代以获得运行时多态；例如，避
 *   免存储 `GameCRTP<SomeType>* ptr` 或将 `GameCRTP<Base>` 当作多态基类
 *   指针来指向不同实例，这会导致切片、未定义行为或错误绑定；
 * - 若需要静态与动态多态混合，请将 CRTP 与小型运行时多态接口结
 *   合使用（参考本目录中的混合示例）；
 * - 注意模板实例化成本：当不同 Derived 类型数量可控且性能优先时
 *   使用 CRTP；
 * - 将 CRTP 接口保持精简，并在文档中声明派生类必须实现的方法
 *   （例如 initialize、makeMove、isFinished、printWinner）。
 *
 * 适用场景示例：
 * - 性能敏感的内部循环（如数学内核、序列化）；
 * - 编译期已知的策略/混入组合。
 */
template <typename Derived> class GameCRTP {
  private:
    // Helper to access the derived implementation (static polymorphism)
    Derived & derived() { return static_cast<Derived &>(*this); }

  public:
    // Template method: algorithm skeleton that delegates primitive operations
    void run() {
        derived().initialize();
        while (!derived().isFinished()) {
            derived().makeMove();
        }
        derived().printWinner();
    }
};

class ChessCRTP : public GameCRTP<ChessCRTP> {
  private:
    // state
    int moves    = 0;
    int maxMoves = 2;

  public:
    void initialize() { fmt::print("[CRTP] Chess initialized.\n"); }

    void makeMove() {
        fmt::print("[CRTP] Chess move\n");
        ++moves;
    }

    bool isFinished() { return moves >= maxMoves; }

    void printWinner() { fmt::print("[CRTP] Chess winner\n"); }
};

class SoccerCRTP : public GameCRTP<SoccerCRTP> {
  private:
    // state
    int minutes    = 0;
    int maxMinutes = 1;

  public:
    void initialize() { fmt::print("[CRTP] Soccer initialized.\n"); }

    void makeMove() {
        fmt::print("[CRTP] Soccer minute\n");
        ++minutes;
    }

    bool isFinished() { return minutes >= maxMinutes; }

    void printWinner() { fmt::print("[CRTP] Soccer winner\n"); }
};

int main() {
    ChessCRTP c;
    fmt::print("Running [CRTP] Chess:\n");
    c.run();

    fmt::print("\n");

    SoccerCRTP s;
    fmt::print("Running [CRTP] Soccer:\n");
    s.run();

    return 0;
}
