/* shape.c - 使用虚函数表实现 C 语言中的多态
 * 本文件实现了 Circle、Rectangle 和 Triangle 三个“类”
 */

#include "shape.h"
#include <math.h>

/* ========== Circle 实现 ========== */

/* Circle 的虚函数表实现 */
static void Circle_draw_impl(const Shape* self) {
    const Circle* circle = (const Circle*)self;
    printf("  [Circle] Center: (%.2f, %.2f), Radius: %.2f\n",
           circle->center_x, circle->center_y, circle->radius);
}

static double Circle_area_impl(const Shape* self) {
    const Circle* circle = (const Circle*)self;
    return M_PI * circle->radius * circle->radius;
}

static void Circle_destroy_impl(Shape* self) {
    Circle* circle = (Circle*)self;
    printf("  Destroying Circle at (%.2f, %.2f)\n", 
           circle->center_x, circle->center_y);
    free(circle);
}

static const char* Circle_get_type_impl(const Shape* self) {
    (void)self;  /* Unused parameter */
    return "Circle";
}

/* Circle vtable - static constant */
static const ShapeVTable Circle_vtable = {
    .draw = Circle_draw_impl,
    .area = Circle_area_impl,
    .destroy = Circle_destroy_impl,
    .get_type = Circle_get_type_impl
};

/* Circle constructor */
Circle* Circle_new(double x, double y, double radius) {
    Circle* circle = (Circle*)malloc(sizeof(Circle));
    if (circle) {
            circle->base.vtable = &Circle_vtable;  /* 设置虚函数表指针 */
        circle->center_x = x;
        circle->center_y = y;
        circle->radius = radius;
    }
    return circle;
}

void Circle_delete(Circle* circle) {
    if (circle) {
        free(circle);
    }
}

/* ========== Rectangle 实现 ========== */

static void Rectangle_draw_impl(const Shape* self) {
    const Rectangle* rect = (const Rectangle*)self;
    printf("  [Rectangle] Position: (%.2f, %.2f), Width: %.2f, Height: %.2f\n",
           rect->x, rect->y, rect->width, rect->height);
}

static double Rectangle_area_impl(const Shape* self) {
    const Rectangle* rect = (const Rectangle*)self;
    return rect->width * rect->height;
}

static void Rectangle_destroy_impl(Shape* self) {
    Rectangle* rect = (Rectangle*)self;
    printf("  Destroying Rectangle at (%.2f, %.2f)\n", rect->x, rect->y);
    free(rect);
}

static const char* Rectangle_get_type_impl(const Shape* self) {
    (void)self;
    return "Rectangle";
}

/* Rectangle 的虚函数表 */
static const ShapeVTable Rectangle_vtable = {
    .draw = Rectangle_draw_impl,
    .area = Rectangle_area_impl,
    .destroy = Rectangle_destroy_impl,
    .get_type = Rectangle_get_type_impl
};

/* Rectangle 的构造函数 */
Rectangle* Rectangle_new(double x, double y, double width, double height) {
    Rectangle* rect = (Rectangle*)malloc(sizeof(Rectangle));
    if (rect) {
        rect->base.vtable = &Rectangle_vtable;
        rect->x = x;
        rect->y = y;
        rect->width = width;
        rect->height = height;
    }
    return rect;
}

void Rectangle_delete(Rectangle* rect) {
    if (rect) {
        free(rect);
    }
}

/* ========== Triangle 实现 ========== */

static void Triangle_draw_impl(const Shape* self) {
    const Triangle* triangle = (const Triangle*)self;
    printf("  [Triangle] Vertices: (%.2f,%.2f), (%.2f,%.2f), (%.2f,%.2f)\n",
           triangle->x1, triangle->y1,
           triangle->x2, triangle->y2,
           triangle->x3, triangle->y3);
}

static double Triangle_area_impl(const Shape* self) {
    const Triangle* t = (const Triangle*)self;
    /* 使用海伦公式或叉乘法计算面积 */
    /* 面积 = 0.5 * |x1(y2-y3) + x2(y3-y1) + x3(y1-y2)| */
    return fabs(0.5 * (t->x1 * (t->y2 - t->y3) + 
                       t->x2 * (t->y3 - t->y1) + 
                       t->x3 * (t->y1 - t->y2)));
}

static void Triangle_destroy_impl(Shape* self) {
    Triangle* triangle = (Triangle*)self;
    printf("  Destroying Triangle\n");
    free(triangle);
}

static const char* Triangle_get_type_impl(const Shape* self) {
    (void)self;
    return "Triangle";
}

/* Triangle vtable */
static const ShapeVTable Triangle_vtable = {
    .draw = Triangle_draw_impl,
    .area = Triangle_area_impl,
    .destroy = Triangle_destroy_impl,
    .get_type = Triangle_get_type_impl
};

/* Triangle constructor */
Triangle* Triangle_new(double x1, double y1, double x2, double y2, double x3, double y3) {
    Triangle* triangle = (Triangle*)malloc(sizeof(Triangle));
    if (triangle) {
        triangle->base.vtable = &Triangle_vtable;
        triangle->x1 = x1;
        triangle->y1 = y1;
        triangle->x2 = x2;
        triangle->y2 = y2;
        triangle->x3 = x3;
        triangle->y3 = y3;
    }
    return triangle;
}

void Triangle_delete(Triangle* triangle) {
    if (triangle) {
        free(triangle);
    }
}
