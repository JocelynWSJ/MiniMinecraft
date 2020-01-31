#ifndef NPCSYSTEM_H
#define NPCSYSTEM_H
#include "drawable.h"
#include "la.h"
#include "smartpointerhelp.h"
#include "terrain.h"

// a box represented by 8 vertices, can deform
class Hexahedron
{
public:
    //    2     3
    // 6     7
    //                y
    //    0     1     o x
    // 4     5      z
    glm::vec3 pos[8];
    glm::vec3 color;
private:
    Hexahedron(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
               glm::vec3 p4, glm::vec3 p5, glm::vec3 p6, glm::vec3 p7,
               glm::vec3 co): color(co) {
        pos[0] = p0; pos[1] = p1; pos[2] = p2; pos[3] = p3;
        pos[4] = p4; pos[5] = p5; pos[6] = p6; pos[7] = p7; }
public:
    Hexahedron(float x0, float x1, float y0, float y1, float z0, float z1,
               glm::vec3 co):
        Hexahedron(glm::vec3(min(x0, x1), min(y0, y1), min(z0, z1)),
                   glm::vec3(max(x0, x1), min(y0, y1), min(z0, z1)),
                   glm::vec3(min(x0, x1), max(y0, y1), min(z0, z1)),
                   glm::vec3(max(x0, x1), max(y0, y1), min(z0, z1)),
                   glm::vec3(min(x0, x1), min(y0, y1), max(z0, z1)),
                   glm::vec3(max(x0, x1), min(y0, y1), max(z0, z1)),
                   glm::vec3(min(x0, x1), max(y0, y1), max(z0, z1)),
                   glm::vec3(max(x0, x1), max(y0, y1), max(z0, z1)),
                   co) {}
    Hexahedron(const Hexahedron &other):
        Hexahedron(other.pos[0], other.pos[1], other.pos[2], other.pos[3],
                   other.pos[4], other.pos[5], other.pos[6], other.pos[7],
                   other.color) {}
    // populate vbo information, with this's vertex positions
    void populate(std::vector<GLuint> &idx, std::vector<float> &info) const;
    // stretch to points given index, as a line
    void stretch(int a, int b, float ratio);
    // offset one point to another
    void offset(int move, int base, float ratio);
public:
    static float min(float a, float b) { return a < b ? a : b; }
    static float max(float a, float b) { return a < b ? b : a; }
};

// a part of npc body, basic element to draw
class BodyPart : public Drawable
{
private:
    std::vector<Hexahedron> m_boxs;
public:
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    int m_parent;
public:
    BodyPart(OpenGLContext *context, const glm::vec3 &position,
             const glm::vec3 &rotation, const glm::vec3 &scale, int parent):
        Drawable(context), m_boxs(), m_position(position),
        m_rotation(rotation), m_scale(scale), m_parent(parent) {}
    BodyPart(OpenGLContext *context, const glm::vec3 &position):
        BodyPart(context, position,
                 glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f), -1) {}
    BodyPart(OpenGLContext *context):
        BodyPart(context, glm::vec3(0.f, 0.f, 0.f)) {}
    virtual ~BodyPart() {}
    // set vbo
    void create() override;
    // get local transformation of this body part
    glm::mat4 trans() const;
    // add a box to this body part
    void add(Hexahedron box) { m_boxs.push_back(box); }
public:
};

// what is the npc currently doing
enum Mission : unsigned char
{
    IDLE, MOVE, ROTL, ROTR, JUMP
};

// base class for all npcs
class NPC
{
protected:
    std::vector<BodyPart> m_parts;
    OpenGLContext *m_context;
    Terrain *m_terrain;
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    Hexahedron m_collider;
    Mission m_mission;
    float m_amount;
    float m_blendw;
    float m_blendt;
    bool m_blendlt;
    float m_blendj;
    float m_blenduj;
public:
    NPC(OpenGLContext *context, Terrain *terrain,
        const glm::vec3 &position, const glm::vec3 &rotation,
        const Hexahedron &collider):
        m_parts(), m_context(context), m_terrain(terrain),
        m_position(position), m_rotation(rotation),
        m_collider(collider), m_mission(IDLE), m_amount(0.f),
        m_blendw(0.f), m_blendt(0.f), m_blendlt(false),
        m_blendj(0.f), m_blenduj(0.f) {}
    virtual ~NPC() {}
    // number of body parts in this npc
    unsigned int size() const { return m_parts.size(); }
    // get the body part at a given index
    BodyPart* partAt(unsigned int index);
    // get the transformation of a body part at a given index
    glm::mat4 partTrans(unsigned int index) const;
    // update the movement of this npc
    virtual void update(float deltaTime, float totalTime) = 0;
protected:
    // set vbo for all body parts
    void create();
    // initialize this npc
    virtual void birth() = 0;
    // idle animation
    virtual void idle(float totalTime, float blend) = 0;
    // walk forward animation
    virtual void walk(float totalTime, float blend) = 0;
    // turn around animation
    virtual void turn(float totalTime, float blend, bool left) = 0;
    // jump animation
    virtual void jump(float totalTime, float blend, float up) = 0;
    virtual float moveSpeed() const { return 1.f; }
    virtual float rotateSpeed() const { return 1.f; }
    virtual float jumpSpeed() const { return 1.f; }
    virtual float fallSpeed() const  { return 1.f; }
    virtual float jumpHeight() const { return 1.5f; }
    virtual float floatHeight() const { return 0.5f; }
    virtual float blendSpeed() const { return 3.f; }
    // detected whether there is a collide in positive x direction
    bool notCollidePosX(const glm::mat4 &newtrans) const;
    // get the moving vector on positive x direction
    glm::vec3 vecMoveAlongX(float amount) const;
    // check collision for positive x direction
    bool canMoveAlongX(float amount) const;
    // check whether the npc can jump onto the facing barrier
    bool canPassAlongX(float amount) const;
    // check whether the npc can live in the facing biome
    bool canLiveAlongX(float amount, BiomeType biomeA, BiomeType biomeB) const;
    // actually move the npc forward (positive x)
    void doMoveAlongX(float amount);
    // actually make the npc jump (positive y)
    void doJump(float amount);
    // actually make the npc rotate around y aixs
    void doRotateAlongY(float angle);
    // do vertical collision detection and make the npc fall
    // also check for floating in the water
    bool appendGravity(float deltaTime);
    // let the npc wander around the world in its favorite biomes
    void wander(float deltaTime, float totalTime,
                BiomeType biomeA, BiomeType biomeB);
};

class Ghost : public NPC
{
public:
    Ghost(OpenGLContext *context, Terrain *terrain,
          const glm::vec3 &pos, const glm::vec3 &rot):
        NPC(context, terrain, pos, rot,
            Hexahedron(-0.5f, 0.5f, 0.f, 2.f, -0.5f, 0.5f, glm::vec3()))
    { birth(); create(); }
    virtual ~Ghost() {}
    void update(float deltaTime, float totalTime) override;
private:
    void birth() override;
    void idle(float totalTime, float blend) override;
    void walk(float totalTime, float blend) override;
    void turn(float totalTime, float blend, bool left) override;
    void jump(float totalTime, float blend, float) override;
    float jumpSpeed() const override { return 2.f; }
    float jumpHeight() const override { return 4.5f; }
};

class Penguin : public NPC
{
public:
    Penguin(OpenGLContext *context, Terrain *terrain,
            const glm::vec3 &pos, const glm::vec3 &rot):
        NPC(context, terrain, pos, rot,
            Hexahedron(-0.5f, 0.5f, 0.f, 1.5f, -0.5f, 0.5f, glm::vec3()))
    { birth(); create(); }
    virtual ~Penguin() {}
    void update(float deltaTime, float totalTime) override;
private:
    void birth() override;
    void idle(float, float blend) override;
    void walk(float totalTime, float blend) override;
    void turn(float totalTime, float blend, bool left) override;
    void jump(float totalTime, float blend, float up) override;
    float jumpSpeed() const override { return 3.f; }
    float fallSpeed() const override { return 3.f; }
    float jumpHeight() const override { return 3.5f; }
};

class Fish : public NPC
{
public:
    Fish(OpenGLContext *context, Terrain *terrain,
         const glm::vec3 &pos, const glm::vec3 &rot):
        NPC(context, terrain, pos, rot,
            Hexahedron(-0.5f, 0.5f, 0.f, 1.f, -0.5f, 0.5f, glm::vec3()))
    { birth(); create(); }
    virtual ~Fish() {}
    void update(float deltaTime, float totalTime) override;
private:
    void birth() override;
    void idle(float totalTime, float blend) override;
    void walk(float totalTime, float blend) override;
    void turn(float totalTime, float blend, bool left) override;
    void jump(float totalTime, float blend, float) override;
    float jumpHeight() const override { return 0.f; }
    float blendSpeed() const override { return 1.f; }
};

class Sheep : public NPC
{
public:
    Sheep(OpenGLContext *context, Terrain *terrain,
          const glm::vec3 &pos, const glm::vec3 &rot):
        NPC(context, terrain, pos, rot,
            Hexahedron(-0.7f, 0.7f, 0.f, 2.f, -0.5f, 0.5f, glm::vec3()))
    { birth(); create(); }
    virtual ~Sheep() {}
    void update(float deltaTime, float totalTime) override;
private:
    void birth() override;
    void idle(float, float blend) override;
    void walk(float totalTime, float blend) override;
    void turn(float totalTime, float blend, bool left) override;
    void jump(float, float blend, float up) override;
    float moveSpeed() const override { return 1.5f; }
    float jumpSpeed() const override { return 3.f; }
    float fallSpeed() const override { return 3.f; }
    float jumpHeight() const override { return 2.5f; }
};

class NPCSystem
{
public:
    std::vector<uPtr<NPC>> npcs;
private:
    OpenGLContext *m_context;
    Terrain *m_terrain;
    float m_totalTime;
public:
    NPCSystem(OpenGLContext *context, Terrain *terrain):
        m_context(context), m_terrain(terrain), m_totalTime(0.f) {}
    // place npc in a certain scope
    void birthNPC(const Rect16 &scope);
    // update the movement of all the npcs
    void update(float deltaTime);
    void destroy();
};

#endif // NPCSYSTEM_H
