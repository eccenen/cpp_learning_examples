/* shape.h - 用 C 语言演示多态
 * 本头文件使用函数指针（虚函数表 vtable 模式）定义 Shape 的“接口”
 * 以在纯 C 中实现多态性。
 */

#ifndef C_POLYMORPHISM_SHAPE_H
#define C_POLYMORPHISM_SHAPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 前向声明 */
typedef struct Shape       Shape;
typedef struct ShapeVTable ShapeVTable;

/* 虚函数表结构 - 包含函数指针（类似于 C++ 的 vtable） */
struct ShapeVTable {
    void (*draw)(const Shape * self);
    double (*area)(const Shape * self);
    void (*destroy)(Shape * self);
    const char * (*get_type)(const Shape * self);
};

/* 基类 - Shape */
struct Shape {
    const ShapeVTable * vtable; /* Pointer to virtual table */
};

/* Shape 的“方法” - 这些会分发到虚函数表中的函数 */
static inline void Shape_draw(const Shape * self) {
    if (self && self->vtable && self->vtable->draw) {
        self->vtable->draw(self);
    }
}

static inline double Shape_area(const Shape * self) {
    if (self && self->vtable && self->vtable->area) {
        return self->vtable->area(self);
    }
    return 0.0;
}

static inline void Shape_destroy(Shape * self) {
    if (self && self->vtable && self->vtable->destroy) {
        self->vtable->destroy(self);
    }
}

static inline const char * Shape_get_type(const Shape * self) {
    if (self && self->vtable && self->vtable->get_type) {
        return self->vtable->get_type(self);
    }
    return "Unknown";
}

/* Circle 类 - 继承自 Shape */
typedef struct Circle {
    Shape  base; /* 继承基类Shape */
    double radius;
    double center_x;
    double center_y;
} Circle;

/* Circle 的构造函数与虚函数表 */
Circle * Circle_new(double x, double y, double radius);
void     Circle_delete(Circle * circle);

/* Rectangle 类 - 继承自 Shape */
typedef struct Rectangle {
    Shape  base;
    double width;
    double height;
    double x;
    double y;
} Rectangle;

/* Rectangle 的构造函数与虚函数表 */
Rectangle * Rectangle_new(double x, double y, double width, double height);
void        Rectangle_delete(Rectangle * rect);

/* Triangle 类 - 继承自 Shape */
typedef struct Triangle {
    Shape  base;
    double x1, y1;
    double x2, y2;
    double x3, y3;
} Triangle;

/* Triangle 的构造函数与虚函数表 */
Triangle * Triangle_new(double x1, double y1, double x2, double y2, double x3, double y3);
void       Triangle_delete(Triangle * triangle);

#endif /* C_POLYMORPHISM_SHAPE_H */
