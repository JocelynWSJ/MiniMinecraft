#ifndef RECTANGLE_H
#define RECTANGLE_H
#include <cmath>
#include <QList>

// 2D float-based vector
class Vec2f
{
public:
    float x;
    float z;
public:
    Vec2f(float _x, float _z): x(_x), z(_z) {}
    Vec2f(const Vec2f &v): Vec2f(v.x, v.z) {}
    Vec2f& operator =(const Vec2f &v) { x = v.x; z = v.z; return *this; }
    Vec2f operator /(float f) const;
    // get the length of this vector
    float length() const { return (float)sqrt(x * x + z * z); }
    // get the normalize vector of this, this won't change
    Vec2f normalize() const { return Vec2f(x, z) / length(); }
    // get the radians of the angle between this vector and +x
    float radians() const;
};

// 2D int-based vector
class Vec2
{
public:
    int x;
    int z;
public:
    Vec2(int _x, int _z): x(_x), z(_z) {}
    Vec2(const Vec2 &v): Vec2(v.x, v.z) {}
    Vec2& operator =(const Vec2 &v) { x = v.x; z = v.z; return *this; }
    Vec2 operator +(const Vec2 &v) const { return Vec2(x + v.x, z + v.z); }
    // convert int vector to float vector
    Vec2f vec2f() const { return Vec2f((float)x, (float)z); }
};

// 2D int-based xz-plane rectangle, min and max including
class Rect
{
public:
    int xmin;
    int xmax;
    int zmin;
    int zmax;
public:
    Rect(int x0, int x1, int z0, int z1):
        xmin(min(x0, x1)), xmax(max(x0, x1)),
        zmin(min(z0, z1)), zmax(max(z0, z1)) {}
    Rect(const Rect &r): Rect(r.xmin, r.xmax, r.zmin, r.zmax) {}
    // check whether a point is inside a rectangle
    bool contains(int x, int z) const;
    // check if this rectangle is inside another
    bool isInside(const Rect &other) const;
    // check if this rectangle has intersection with another
    bool hasIntersect(const Rect &other) const;
    // get the intersection rectangle of this and another
    Rect getIntersect(const Rect &other) const;
    // get x-axis middle value
    int xmid() const { return (xmin + xmax) / 2; }
    // get z-axis middle value
    int zmid() const { return (zmin + zmax) / 2; }
    // print the information of this rectangle to std output
    void print() const;
public:
    // get the bounding box of a capsule (2 spheres and 1 cylinder)
    static Rect capsuleBBox(float x0, float z0, float x1, float z1, float r);
    // get the smaller of 2 ints
    static int min(int a, int b) { return a < b ? a : b; }
    // get the larger of 2 ints
    static int max(int a, int b) { return a < b ? b : a; }
    // get the largest int that is smaller than both floats
    static int minf(float a, float b) { return min((int)floorf(a), (int)floorf(b)); }
    // get the smallest int that is larger than both floats
    static int maxf(float a, float b) { return max((int)floorf(a) + 1,
                                                   (int)floorf(b) + 1); }
};

// rectangle that starts at 16n and ends at 16n+15
class Rect16 : public Rect
{
public:
    Rect16(int x, int z):
        Rect(x & -16, (x & -16) + 15, z & -16, (z & -16) + 15) {}
    Rect16(const Rect16 &other):
        Rect(other.xmin, other.xmax, other.zmin, other.zmax) {}
    bool operator ==(const Rect16 &other) const {
        return this->xmin == other.xmin && this->zmin == other.zmin; }
};

// rectangle that starts at 64n and ends at 64n+63
class Rect64 : public Rect
{
public:
    Rect64(int x, int z):
        Rect(x & -64, (x & -64) + 63, z & -64, (z & -64) + 63) {}
    Rect64(const Rect64 &other):
        Rect(other.xmin, other.xmax, other.zmin, other.zmax) {}
    bool operator ==(const Rect64 &other) const {
        return this->xmin == other.xmin && this->zmin == other.zmin; }
public:
    // test and print rect64 class
    static void testRectangle64();
};

// a domain that contains multiple non-repetitive rect64
class Domain
{
public:
    QList<Rect64> rects;
public:
    Domain(): rects() {}
    Domain(const Domain &other): rects(other.rects) {}
    // check if this domain contains a given rect64
    bool has(const Rect64 &rect) const;
    // check is this domain has intersect with another
    bool hasIntersect(const Domain &other) const;
    // add all rect64 that has intersection with given rect to this domain
    void addRect(const Rect &rect);
    // add a rect64 to this domain
    void add64(const Rect64 &rect);
    // add another domain to this
    void addDomain(const Domain &other);
    // print the information of this rectangle to std output
    void print() const;
public:
    // test and print domain class
    static void testDomain();
};

#endif // RECTANGLE_H
