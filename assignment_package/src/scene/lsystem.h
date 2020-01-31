#ifndef LSYSTEM_H
#define LSYSTEM_H
#include <memory>
#include <stack>
#include <QList>
#include <QStack>
#include <QHash>
#include "terrain.h"

// the ratio of river erode range to its width
const float RIVER_ERODE_MULT = 10.f;
// global random seed for rivers
const float RIVER_SEED = 72.5f;

// a symbol of turtle drawing instruction
class Symbol
{
public:
    char action;  // test and print domain class
    bool down;    // whether this segment is going downstream
    float width;  // width of this segment
    float length; // length of this segment
    float angle;  // angle that the turtle will turn
public:
    Symbol(char act, bool d, float w = 0.f, float l = 0.f, float a = 0.f):
        action(act), down(d), width(w), length(l), angle(a) {}
    Symbol(const Symbol &s):
        action(s.action), down(s.down),
        width(s.width), length(s.length), angle(s.angle) {}
};

// a turtle that draws river
class Turtle
{
public:
    float x;   // the current x location
    float z;   // the current z location
    float dir; // the current radians direction, +x is 0
public:
    Turtle(float x0, float z0, float d):
        x(x0), z(z0), dir(d) {}
    Turtle(const Turtle &t):
        x(t.x), z(t.z), dir(t.dir) {}
};

// a branch of a river
class RiverBranch
{
private:
    Turtle m_start;       // branch start position and orientation
    Turtle m_cur;         // current drawing position and orientation
    int m_index;          // current index of symbol
    QList<Symbol> m_path; // the draw path of this branch, which are symbols
    bool m_done;          // has this branch draw itself entirely onto terrain
    Domain m_domain;      // the domain of this branch
    bool m_domainSet;     // has the domain already been computed
    Terrain *m_terrain;   // points to the game's terrain
public:
    RiverBranch(Terrain *t, Turtle s):
        m_start(s), m_cur(s), m_index(0), m_path(), m_done(false),
        m_domain(), m_domainSet(false), m_terrain(t) {}
    Turtle current() const { return m_cur; }
    bool done() const { return m_done; }
    // get end position and orientation of this branch
    Turtle endpos();
    // get the domain of this branch
    Domain domain();
    // append a symbol to the path of this branch
    void append(const Symbol &symbol);
    // reset current turtle to start
    void reset();
    // draw the branch onto terrain in a certain scope
    void draw(const Rect &scope);
private:
    // move one step, isDraw determines whether to draw onto terrain
    // return false when out of range
    bool step(bool isDraw, const Rect &scope = Rect(0, 0, 0, 0));
    // go forward step
    bool stepForward(bool isDraw, const Rect &scope,
                     Turtle &turtle, const Symbol &param);
    // turn left step
    bool stepLeft(bool isDraw, const Rect &scope,
                  Turtle &turtle, const Symbol &param);
    // turn right step
    bool stepRight(bool isDraw, const Rect &scope,
                   Turtle &turtle, const Symbol &param);
    // circle lake step
    bool stepLake(bool isDraw, const Rect &scope,
                  Turtle &turtle, const Symbol &param);
    // draw a line on to terrain
    bool drawLine(const Rect &scope,
                  float x0, float z0, float x1, float z1, float r);
};

// a lsystem river
class River
{
protected:
    Turtle m_initpos;                 // river start position and orientation
    QList<Symbol> m_grammar;          // lsystem grammar of this river
    QList<RiverBranch> m_branches;    // all braches
    std::stack<RiverBranch*> m_stack; // a helper stack when building branches
    Terrain *m_terrain;               // points to the game's terrain
public:
    River(const QList<Symbol> &axiom, Terrain *t, Turtle pos):
        m_initpos(pos), m_grammar(axiom), m_branches(), m_stack(), m_terrain(t) {}
    virtual ~River() {}
    // get end position and orientation of this river
    Turtle endpos();
    // last non-'[]' symbol of this river
    Symbol endsymbol() const;
    // get the domain of this river
    Domain domain();
    // draw the river onto terrain in a certain scope
    bool draw(const Rect &scope);
protected:
    // grammar expand
    virtual void expand(int iteration) = 0;
    // build river branches from grammar
    void build();
};

// a linear river
class LinearRiver : public River
{
public:
    LinearRiver(const QList<Symbol> &axiom, Terrain *terrain, Turtle pos, int iter):
        River(axiom, terrain, pos) { expand(iter); build(); }
    virtual ~LinearRiver() {}
private:
    // linear river grammar expand
    void expand(int iteration) override;
};

// a delta river
class DeltaRiver : public River
{
public:
    DeltaRiver(const QList<Symbol> &axiom, Terrain *terrain, Turtle pos, int iter):
        River(axiom, terrain, pos) { expand(iter); build(); }
    virtual ~DeltaRiver() {}
private:
    // delta river grammar expand
    void expand(int iteration) override;
};

// holds all the rivers
class LSystem
{
private:
    std::vector<std::unique_ptr<River>> m_rivers; // all existing rivers
    Domain m_domain;                              // combined domain of all rivers
    Terrain *m_terrain;                           // points to the game's terrain
public:
    LSystem(Terrain *terrain):
        m_rivers(), m_domain(), m_terrain(terrain) {}
    // draw all rivers in a certain scope, also try to make new rivers
    void update(const Rect &scope);
private:
    // initialize a linear river
    void makeLinearRiver(Turtle pos);
    // initialzie a delta river
    void makeDeltaRiver(Turtle pos, Symbol param);
};

#endif // LSYSTEM_H
