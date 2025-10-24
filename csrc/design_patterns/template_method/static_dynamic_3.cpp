/**
 * @brief 类型擦除+CRTP模板方法模式示例：静态多态实现
 */

#include <iostream>
#include <memory>
#include <typeinfo>
#include <vector>

// === 第一步：静态多态基础（CRTP）===
template <typename Derived> class DrawableCRTP {
  public:
    // 编译期多态方法
    void draw() const { static_cast<const Derived *>(this)->drawImpl(); }

    double calculateArea() const { return static_cast<const Derived *>(this)->areaImpl(); }

    // 通用功能 - 编译期解析
    void drawWithDetails() const {
        std::cout << "Drawing: ";
        draw();
        std::cout << "Area: " << calculateArea() << std::endl;
    }

    // 获取类型信息（编译期）
    const std::type_info & type() const { return typeid(Derived); }
};

// === 第二步：动态多态包装（类型擦除）===
class DrawableInterface {
  public:
    virtual ~DrawableInterface()                                       = default;
    virtual void                               draw() const            = 0;
    virtual double                             calculateArea() const   = 0;
    virtual void                               drawWithDetails() const = 0;
    virtual std::unique_ptr<DrawableInterface> clone() const           = 0;
};

// 类型擦除包装器
template <typename T> class DrawableWrapper : public DrawableInterface {
  private:
    T object_;

  public:
    DrawableWrapper(T obj) : object_(std::move(obj)) {}

    void draw() const override {
        object_.draw(); // 静态多态调用
    }

    double calculateArea() const override {
        return object_.calculateArea(); // 静态多态调用
    }

    void drawWithDetails() const override {
        object_.drawWithDetails(); // 使用CRTP的通用功能
    }

    std::unique_ptr<DrawableInterface> clone() const override {
        return std::make_unique<DrawableWrapper>(object_);
    }
};

// === 第三步：统一管理类 ===
class AnyDrawable {
  private:
    std::unique_ptr<DrawableInterface> pimpl_;

  public:
    // 模板构造函数 - 接受任何符合CRTP接口的类型
    template <typename T>
    AnyDrawable(T obj) : pimpl_(std::make_unique<DrawableWrapper<T>>(std::move(obj))) {
        static_assert(std::is_base_of_v<DrawableCRTP<T>, T>, "T must inherit from DrawableCRTP<T>");
    }

    // 动态多态接口
    void draw() const { pimpl_->draw(); }

    double calculateArea() const { return pimpl_->calculateArea(); }

    void drawWithDetails() const { pimpl_->drawWithDetails(); }

    // 拷贝支持
    AnyDrawable(const AnyDrawable & other) : pimpl_(other.pimpl_->clone()) {}

    AnyDrawable & operator=(const AnyDrawable & other) {
        if (this != &other) {
            pimpl_ = other.pimpl_->clone();
        }
        return *this;
    }

    // 移动支持
    AnyDrawable(AnyDrawable &&)             = default;
    AnyDrawable & operator=(AnyDrawable &&) = default;
};

// === 具体实现类 ===
class Circle : public DrawableCRTP<Circle> {
  private:
    double radius_;
  public:
    Circle(double r) : radius_(r) {}

    void drawImpl() const { std::cout << "Circle(r=" << radius_ << ")"; }

    double areaImpl() const { return 3.14159 * radius_ * radius_; }
};

class Rectangle : public DrawableCRTP<Rectangle> {
  private:
    double width_, height_;
  public:
    Rectangle(double w, double h) : width_(w), height_(h) {}

    void drawImpl() const { std::cout << "Rectangle(" << width_ << "x" << height_ << ")"; }

    double areaImpl() const { return width_ * height_; }
};

// === 使用示例 ===
void advanced_mixed_example() {
    std::vector<AnyDrawable> shapes;

    // 添加不同类型的对象
    shapes.emplace_back(Circle(5.0));
    shapes.emplace_back(Rectangle(4.0, 6.0));
    shapes.emplace_back(Circle(2.5));

    // 统一管理（动态多态的优势）
    std::cout << "=== Drawing all shapes ===" << std::endl;
    for (const auto & shape : shapes) {
        shape.drawWithDetails(); // 内部使用静态多态的优化实现
    }

    // 计算总面积
    double totalArea = 0;
    for (const auto & shape : shapes) {
        totalArea += shape.calculateArea();
    }
    std::cout << "Total area: " << totalArea << std::endl;
}

int main() {
    std::cout << "=== Advanced Mixed Polymorphism Demo ===\n\n";
    advanced_mixed_example();
    return 0;
}
