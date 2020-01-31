#include "rectangle.h"
#include <iostream>

Vec2f Vec2f::operator /(float f) const {
    if (f == 0.f) return Vec2f(1.f, 0.f);
    else return Vec2f(x / f, z / f);
}

// get the radians of the angle between this vector and +x
float Vec2f::radians() const {
    Vec2f norm = normalize();
    float acos = (float)std::acos(norm.x);
    return norm.z < 0 ? -acos : acos;
}

// check whether a point is inside a rectangle
bool Rect::contains(int x, int z) const {
    return x >= this->xmin && x <= this->xmax && z >= this->zmin && z <= this->zmax;
}

// check if this rectangle is inside another
bool Rect::isInside(const Rect &other) const {
    return this->xmin >= other.xmin && this->xmax <= other.xmax &&
           this->zmin >= other.zmin && this->zmax <= other.zmax;
}

// check if this rectangle has intersection with another
bool Rect::hasIntersect(const Rect &other) const {
    if (this->xmin > other.xmax || this->xmax < other.xmin ||
        this->zmin > other.zmax || this->zmax < other.zmin) {
        return false;
    } else {
        return true;
    }
}

// get the intersection rectangle of this and another
Rect Rect::getIntersect(const Rect &other) const {
    return Rect(max(this->xmin, other.xmin), min(this->xmax, other.xmax),
                     max(this->zmin, other.zmin), min(this->zmax, other.zmax));
}

// print the information of this rectangle to std output
void Rect::print() const {
    std::cout << xmin << "~" << xmax << " " << zmin << "~" << zmax << std::endl;
}

// get the bounding box of a capsule (2 spheres and 1 cylinder)
Rect Rect::capsuleBBox(float x0, float z0, float x1, float z1, float r) {
    return Rect(minf(x0 - r, x1 - r), maxf(x0 + r, x1 + r),
                     minf(z0 - r, z1 - r), maxf(z0 + r, z1 + r));
}

// test and print rect64 class
void Rect64::testRectangle64() {
    Rect64 r0(0, -64);
    std::cout << "make r0(0, -64)" << std::endl;
    r0.print();
    Rect64 r1(135, 64);
    std::cout << "make r1(135, 64)" << std::endl;
    r1.print();
    Rect64 r2(-128, -157);
    std::cout << "make r2(-128, -157)" << std::endl;
    r2.print();
}

// check if this domain contains a given rect64
bool Domain::has(const Rect64 &rect) const {
    for (int i = 0; i < rects.size(); i++) {
        if (rects[i] == rect) return true;
    }
    return false;
}

// check is this domain has intersect with another
bool Domain::hasIntersect(const Domain &other) const {
    for (int i = 0; i < other.rects.size(); i++) {
        if (has(other.rects[i])) return true;
    }
    return false;
}

// add all rect64 that has intersection with given rect to this domain
void Domain::addRect(const Rect &rect) {
    add64(Rect64(rect.xmin, rect.zmin));
    add64(Rect64(rect.xmin, rect.zmax));
    add64(Rect64(rect.xmax, rect.zmin));
    add64(Rect64(rect.xmax, rect.zmax));
}

// add a rect64 to this domain
void Domain::add64(const Rect64 &rect) {
    if (!has(rect)) rects.push_back(rect);
}

// add another domain to this
void Domain::addDomain(const Domain &other) {
    for (int i = 0; i < other.rects.size(); i++) add64(other.rects[i]);
}

// print the information of this rectangle to std output
void Domain::print() const {
    for (int i = 0; i < rects.size(); i++) rects[i].print();
}

// test and print domain class
void Domain::testDomain() {
    Domain domain;
    std::cout << "add Rectangle(12, 63, 3, 63)" << std::endl;
    domain.addRect(Rect(12, 63, 3, 63));
    domain.print();
    std::cout << "add Rectangle(33, 42, 59, 64)" << std::endl;
    domain.addRect(Rect(33, 42, 59, 64));
    domain.print();
    std::cout << "add Rectangle(-15, 0, -30, 0)" << std::endl;
    domain.addRect(Rect(-15, 0, -30, 0));
    domain.print();
}
