#include "lsystem.h"
#include <iostream>

// get end position and orientation of this branch
Turtle RiverBranch::endpos() {
    while (m_index < m_path.size()) {
        step(false);
    }
    return m_cur;
}

// get the domain of this branch
Domain RiverBranch::domain() {
    if (!m_domainSet) {
        reset();
        while (m_index < m_path.size()) {
            step(false);
        }
        m_domainSet = true;
    }
    return m_domain;
}

// append a symbol to the path of this branch
void RiverBranch::append(const Symbol &symbol) {
    m_path.push_back(symbol);
    step(false);
}

// reset current turtle to start
void RiverBranch::reset() {
    m_cur = m_start;
    m_index = 0;
}

// draw the branch onto terrain in a certain scope
void RiverBranch::draw(const Rect &scope) {
    reset();
    m_done = true;
    while (m_index < m_path.size()) {
        if (!step(true, scope)) {
            m_done = false;
        }
    }
}

// move one step, isDraw determines whether to draw onto terrain
// return false when out of range
bool RiverBranch::step(bool isDraw, const Rect &scope) {
    if (m_index < 0 || m_index >= m_path.size()) {
        return true;
    }
    bool inRange = true;
    Turtle end = m_cur;
    switch (m_path[m_index].action) {
    case 'F': // draw forward
        if (!stepForward(isDraw, scope, end, m_path[m_index])) inRange = false;
        break;
    case 'L': // rotate left
        if (!stepLeft(isDraw, scope, end, m_path[m_index])) inRange = false;
        break;
    case 'R': // rotate right
        if (!stepRight(isDraw, scope, end, m_path[m_index])) inRange = false;
        break;
    case 'K': // draw lake
        if(!stepLake(isDraw, scope, end, m_path[m_index])) inRange = false;
        break;
    default: // other action, do nothing
        break;
    }
    m_cur = end;
    m_index++;
    return inRange;
}

// go forward step
bool RiverBranch::stepForward(bool isDraw, const Rect &scope,
                              Turtle &turtle, const Symbol &param) {
    bool inRange = true;
    float dx = param.length * cosf(turtle.dir);
    float dz = param.length * sinf(turtle.dir);
    float x = turtle.x + dx;
    float z = turtle.z + dz;
    if (isDraw) {
        inRange = drawLine(scope, turtle.x, turtle.z, x, z, param.width / 2.f);
    }
    if (!m_domainSet) {
        m_domain.addRect(Rect::capsuleBBox(turtle.x, turtle.z, x, z,
                                                param.width / 2.f * RIVER_ERODE_MULT));
    }
    turtle.x = x;
    turtle.z = z;
    return inRange;
}

// turn left step
bool RiverBranch::stepLeft(bool isDraw, const Rect &scope,
                           Turtle &turtle, const Symbol &param) {
    turtle.dir += param.angle;
    return stepForward(isDraw, scope, turtle, param);
}

// turn right step
bool RiverBranch::stepRight(bool isDraw, const Rect &scope,
                            Turtle &turtle, const Symbol &param) {
    turtle.dir -= param.angle;
    return stepForward(isDraw, scope, turtle, param);
}

// circle lake step
bool RiverBranch::stepLake(bool isDraw, const Rect &scope,
                           Turtle &turtle, const Symbol &param) {
    return stepForward(isDraw, scope, turtle, param);
}

// draw a line on to terrain
bool RiverBranch::drawLine(const Rect &scope,
                           float x0, float z0, float x1, float z1, float r) {
    if (r <= 0) return true;
    Rect bbox = Rect::capsuleBBox(x0, z0, x1, z1, r * RIVER_ERODE_MULT);
    if (!bbox.hasIntersect(scope)) return false;
    bool inRange = true;
    if (!bbox.isInside(scope)) inRange = false;
    bbox = bbox.getIntersect(scope);
    for (int i = bbox.xmin; i <= bbox.xmax; i++) {
        for (int j = bbox.zmin; j <= bbox.zmax; j++) {
            float x = i + 0.5f;
            float z = j + 0.5f;
            // compute distance from point to line segment
            float dist2 = 0;
            if (noise::sqrDist2D(x1 - x0, z1 - z0) <= 0.f) {
                dist2 = noise::sqrDist2D(x - x0, z - z0);
            } else if (noise::dot2D(x1 - x0, z1 - z0, x - x0, z - z0) < 0) {
                dist2 = noise::sqrDist2D(x - x0, z - z0);
            } else if (noise::dot2D(x0 - x1, z0 - z1, x - x1, z - z1) < 0) {
                dist2 = noise::sqrDist2D(x - x1, z - z1);
            } else {
                float area = noise::area2D(x0 - x, z0 - z, x1 - x, z1 - z);
                dist2 = 4.f * area * area / noise::sqrDist2D(x1 - x0, z1 - z0);
            }
            // carve terrain based on distance
            float erodeRange = r * RIVER_ERODE_MULT;
            float t = (float)std::sqrt(dist2) / erodeRange;
            if (t <= 1.f) {
                float top = erodeRange * 1.5f;
                float bottom = -r;
                int restY = (int)floorf(noise::mixCubic(bottom, top, t)) + 128;
                m_terrain->waterErode(i, j, restY);
            }
        }
    }
    return inRange;
}

// get end position and orientation of this river
Turtle River::endpos() {
    if (m_branches.size() < 1) {
        return m_initpos;
    } else {
        return m_branches[0].endpos();
    }
}

// last non-'[]' symbol of this river
Symbol River::endsymbol() const {
    for (int i = m_grammar.size() - 1; i >= 0; i--) {
        if (m_grammar[i].action != ']' && m_grammar[i].action != '[') {
            return m_grammar[i];
        }
    }
    return Symbol(']', true);
}

// get the domain of this river
Domain River::domain() {
    Domain domain;
    for (int i = 0; i < m_branches.size(); i++) {
        domain.addDomain(m_branches[i].domain());
    }
    return domain;
}

// draw the river onto terrain in a certain scope
bool River::draw(const Rect &scope) {
    if (m_branches.size() <= 0) {
        return false;
    }
    for (int i = 0; i < m_branches.size(); i++) {
        m_branches[i].draw(scope);
        if (m_branches[i].done()) {
            m_branches.removeAt(i);
            i--;
        }
    }
    return true;
}

// build river branches from grammar
void River::build() {
    for (int i = 0; i < m_grammar.size(); i++) {
        Symbol symbol = m_grammar.at(i);
        if (symbol.action == '[') {
            if (m_stack.size() <= 0) {
                m_branches.push_back(RiverBranch(m_terrain, m_initpos));
            } else {
                m_branches.push_back(RiverBranch(m_terrain,
                                                 m_stack.top()->current()));
            }
            m_stack.push(&(m_branches[m_branches.size() - 1]));
        } else if (symbol.action == ']') {
            m_stack.pop();
        } else {
            m_stack.top()->append(symbol);
        }
    }
}

// linear river gramar expand
void LinearRiver::expand(int iteration) {
    QList<Symbol> old_grammar;
    for (; iteration > 0; iteration--) {
        old_grammar.clear();
        old_grammar.append(m_grammar);
        m_grammar.clear();
        for (int i = 0; i < old_grammar.size(); i++) {
            Symbol symbol = old_grammar[i];
            float seed = (m_initpos.x + m_initpos.z) *
                         noise::rand1D(float(i)) *
                         noise::rand1D(float(iteration)) + RIVER_SEED;
            float rand = noise::rand1D(seed);
            float w = symbol.width;
            float l = symbol.length;
            float dw = symbol.down ? 1.2f : 0.9f;
            float dl = symbol.down ? 1.05f : 0.95f;
            float angle = noise::randRange(0.1, 0.5, seed + 12.3);
            if (symbol.action == 'B') {
                if (iteration >= 2 && iteration <= 4) {
                    if (rand < 0.5f) {
                        m_grammar.push_back(Symbol('[', false));
                        m_grammar.push_back(Symbol('L', false, w, l, 3.f));
                        m_grammar.push_back(Symbol('F', false, w, l, 0.f));
                        m_grammar.push_back(Symbol('R', false, w, l, angle));
                        m_grammar.push_back(Symbol('F', false, w, l, 0.f));
                        m_grammar.push_back(Symbol('L', false, w, l, angle));
                        m_grammar.push_back(Symbol(']', false));
                    } else {
                        m_grammar.push_back(Symbol('[', false));
                        m_grammar.push_back(Symbol('R', false, w, l, 3.f));
                        m_grammar.push_back(Symbol('F', false, w, l, 0.f));
                        m_grammar.push_back(Symbol('L', false, w, l, angle));
                        m_grammar.push_back(Symbol('F', false, w, l, 0.f));
                        m_grammar.push_back(Symbol('R', false, w, l, angle));
                        m_grammar.push_back(Symbol(']', false));
                    }
                }
            } else if (symbol.action == 'F') {
                m_grammar.push_back(symbol);
                if (symbol.down) {
                    if (rand < 0.7f) {
                        m_grammar.push_back(Symbol('B', true, w * 0.6f, l * 0.8f, 0.f));
                        m_grammar.push_back(Symbol('F', true, w * dw, l * dl, 0.f));
                    } else if (rand < 0.85f) {
                        m_grammar.push_back(Symbol('L', true, w * dw, l * dl, angle));
                    } else {
                        m_grammar.push_back(Symbol('R', true, w * dw, l * dl, angle));
                    }
                } else {
                    if (rand < 0.4f) {
                        m_grammar.push_back(Symbol('L', false, w * dw, l * dl, angle));
                        m_grammar.push_back(Symbol('F', false, w * dw, l * dl, 0.f));
                        m_grammar.push_back(Symbol('R', false, w * dw, l * dl, angle));
                    } else if (rand < 0.8f) {
                        m_grammar.push_back(Symbol('R', false, w * dw, l * dl, angle));
                        m_grammar.push_back(Symbol('F', false, w * dw, l * dl, 0.f));
                        m_grammar.push_back(Symbol('L', false, w * dw, l * dl, angle));
                    } else {
                        m_grammar.push_back(Symbol('F', false, w * dw, l * dl, 0.f));
                        m_grammar.push_back(Symbol('F', false, w * dw, l * dl, 0.f));
                    }
                }
            } else if (symbol.action == 'L') {
                m_grammar.push_back(symbol);
                if (!symbol.down) {
                    m_grammar.push_back(Symbol('F', symbol.down, w * dw, l * dl, 0.f));
                }
                if (rand < 0.5f) {
                    m_grammar.push_back(Symbol('R', symbol.down, w * dw, l * dl, angle));
                } else if (rand < 0.6f) {
                    m_grammar.push_back(Symbol('L', symbol.down, w * dw, l * dl, angle));
                } else {
                    m_grammar.push_back(Symbol('F', symbol.down, w * dw, l * dl, 0.f));
                }
            } else if (symbol.action == 'R') {
                m_grammar.push_back(symbol);
                if (rand < 0.5f) {
                    m_grammar.push_back(Symbol('L', symbol.down, w * dw, l * dl, angle));
                } else if (rand < 0.6f) {
                    m_grammar.push_back(Symbol('R', symbol.down, w * dw, l * dl, angle));
                } else {
                    m_grammar.push_back(Symbol('F', symbol.down, w * dw, l * dl, 0.f));
                }
            } else {
                m_grammar.push_back(symbol);
            }
        }
    }
}

// delta river grammar expand
void DeltaRiver::expand(int iteration) {
    QList<Symbol> old_grammar;
    for (int iter = iteration; iter > 0; iter--) {
//#define PRINT_GRAMMAR
#ifdef PRINT_GRAMMAR
        for (int i = 0; i < m_grammar.size(); i++) {
            std::cout << m_grammar[i].action;
        }
        std::cout << std::endl << std::endl;
#endif
        old_grammar.clear();
        old_grammar.append(m_grammar);
        m_grammar.clear();
        for (int i = 0; i < old_grammar.size(); i++) {
            Symbol symbol = old_grammar[i];
            Symbol next = Symbol(' ', true);
            if (i < old_grammar.size() - 1) next = old_grammar[i + 1];
            float seed = (m_initpos.x + m_initpos.z) *
                         noise::rand1D(float(i)) *
                         noise::rand1D(float(iter)) + RIVER_SEED;
            float rand = noise::rand1D(seed);
            float w = symbol.width;
            float l = symbol.length;
            float dw = 0.95f;
            float dl = 1.05f;
            float angle = noise::randRange(0.3, 0.9, seed + 12.3);
            if (symbol.action == 'B') {
                if (iter >= 2) {
                    m_grammar.push_back(Symbol('[', true));
                    m_grammar.push_back(Symbol('L', true, w, l, 0.7f));
                    m_grammar.push_back(Symbol('R', true, w * dw, l * dl, angle));
                    m_grammar.push_back(Symbol('F', true, w * dw, l * dl, 0.f));
                    m_grammar.push_back(Symbol(']', true));
                    m_grammar.push_back(Symbol('[', true));
                    m_grammar.push_back(Symbol('R', true, w, l, 0.7f));
                    m_grammar.push_back(Symbol('L', true, w * dw, l * dl, angle));
                    m_grammar.push_back(Symbol('F', true, w * dw, l * dl, 0.f));
                    m_grammar.push_back(Symbol(']', true));
                }
            } else if (symbol.action == 'G') {
                if (rand < 0.2) {
                    m_grammar.push_back(Symbol('G', true, w * dw, l * dl, 0.f));
                    m_grammar.push_back(Symbol('G', true, w * dw, l * dl, 0.f));
                } else if (rand < 0.6) {
                    m_grammar.push_back(Symbol('F', true, w * dw, l * dl, 0.f));
                } else if (rand < 0.8) {
                    m_grammar.push_back(Symbol('L', true, w * dw, l * dl, angle));
                    m_grammar.push_back(Symbol('R', true, w * dw, l * dl, angle));
                } else {
                    m_grammar.push_back(Symbol('R', true, w * dw, l * dl, angle));
                    m_grammar.push_back(Symbol('L', true, w * dw, l * dl, angle));
                }
            } else if (symbol.action == 'F') {
                if (rand < (float)iter / (float)iteration * 0.8f) {
                    m_grammar.push_back(Symbol('B', true, w * 0.8f, l * dl, 0.f));
                } else {
                    m_grammar.push_back(Symbol('G', true, w * dw, l * dl, 0.f));
                }
            } else {
                m_grammar.push_back(symbol);
            }
        }
    }
}

// draw all rivers in a certain scope, also try to make new rivers
void LSystem::update(const Rect &scope) {
    for (unsigned int i = 0; i < m_rivers.size(); i++) {
        m_rivers[i]->draw(scope);
    }
    Rect64 scope64 = Rect64(scope.xmid(), scope.zmid());
    if (!m_domain.has(scope64)) {
        int xmid = scope64.xmid();
        int zmid = scope64.zmid();
        // determine direction by seeking blank space around
        int dx[8] = {-64, 0, 64, -64, 64, -64, 0, 64};
        int dz[8] = {-64, -64, -64, 0, 0, 64, 64, 64};
        Vec2 total(0, 0);
        for (int i = 0; i < 8; i++) {
            int xi = xmid + dx[i];
            int zi = zmid + dz[i];
            if (!m_terrain->explored(Rect64(xi, zi))) {
                total = total + Vec2(dx[i], dz[i]);
            }
        }
        float dir = total.vec2f().radians();
        float posx = (float)xmid;
        float posz = (float)zmid;
        Turtle pos(posx, posz, dir);
        makeLinearRiver(pos);
        River *linearRiver = m_rivers[m_rivers.size() - 1].get();
        // connect linear river and delta river
        makeDeltaRiver(linearRiver->endpos(), linearRiver->endsymbol());
        River *deltaRiver = m_rivers[m_rivers.size() - 1].get();
        // check whether there is a conflict
        Domain domain;
        domain.addDomain(linearRiver->domain());
        domain.addDomain(deltaRiver->domain());
        if (m_terrain->explored(domain, scope64) ||
            m_domain.hasIntersect(domain)) {
            m_rivers.pop_back();
            m_rivers.pop_back();
        } else {
            linearRiver->draw(scope);
            deltaRiver->draw(scope);
            m_domain.addDomain(domain);
        }
    }
//#define TEST_RECTANGLES
#ifdef TEST_RECTANGLES
    Rectangle64::testRectangle64();
    Domain::testDomain();
#endif
}

// initialize a linear river
void LSystem::makeLinearRiver(Turtle pos) {
    QList<Symbol> axiom;
    axiom.push_back(Symbol('[', true));
    axiom.push_back(Symbol('F', true, 2.f, 3.f, 0.f));
    axiom.push_back(Symbol(']', true));
    m_rivers.push_back(std::make_unique<LinearRiver>(axiom, m_terrain, pos, 6));
}

// initialzie a delta river
void LSystem::makeDeltaRiver(Turtle pos, Symbol param) {
    QList<Symbol> axiom;
    axiom.push_back(Symbol('[', true));
    axiom.push_back(Symbol('B', true, param.width, param.length, 0.f));
    axiom.push_back(Symbol(']', true));
    m_rivers.push_back(std::make_unique<DeltaRiver>(axiom, m_terrain, pos, 20));
}
